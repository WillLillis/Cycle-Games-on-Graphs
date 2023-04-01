/*
*
* - This file will serve as the "work horse" for our game playing code
* - In this header we will define the functions that take in and play the 
* MAC and AAC games, given a common input form (reference to a std::vector)
* 
* - Going to begin by copying over a bunch of my crap code from the first 
* pass on this project, and then cleaning it up
* 
* - We could make this an object oriented project with a bunch of classes and whatnot, 
* but that's not something I'm not super comfortable with, and also seems like it would 
* be an additional hurdle to future research interns looking to contribute to the code base
* 
* As a result of this (potentially misguided) decision, the code is going to rewritten 
* in a loose style that doesn't focus on object oriented things like classes, and hopefully that's ok
* 
* The max value of an uint16_t is 65,535
* I don't see us realistically looking at graphs that would make my use of a uint16_t to 
* track the edges/ nodes insufficient, so we'll go with that data type for now
* 
* Anwyways, here goes...
*
*/

#pragma once
#include <cstdint>
#include "Adjacency_Matrix.h"
#include <fstream>
#include <cstdarg>
#include <cassert>
#include <thread>
#include <vector>
#include <cstring> // needed for memset

// We'll define the game state in the following manner
typedef int_fast16_t GAME_STATE;
#define WIN_STATE		1
#define LOSS_STATE		0
#define ERROR_STATE		-1 // Error reporting
#define KILL_STATE		-2 // For multithreaded version
#define RUN_STATE		-3 // ^

// maybe replace?
//enum class GAME_STATE1 : int_fast16_t
//{
//	WIN_STATE1,
//
//
//};

typedef uint_fast8_t PLAYER_SIDE;
#define PLAYER_1		1
#define PLAYER_2		2

// do we want a mark_edge function to mark edges as used/ not used, that 
// will automatically get the i,j and j,i entry in the matrix, so that 
// we don't have to manually do that everytime?

/*
* 
* The edge and node usage matrices seem specific enough to these functions that they shouldn't have their own files
* As a result, we'll just #define some values for their use below for code clarity
* 
*/
#define USED		1
#define NOT_USED	0

/****************************************************************************
* fprint_indent
*
* - prints the specified number of tabs to the specified file stream
* - used as indents to indicate the level of recursion in the output files 
* of loud runs
*
* Parameters :
* - output : the file stream to print the tabs to
* - num_indent : the number of tabs to print
*
* Returns :
* - none
****************************************************************************/
// move to Misc.h?
void fprint_indent(FILE* __restrict output, const uint_fast16_t num_indent)
{
	if (output == NULL) {
		DISPLAY_ERR(true,
			"The supplied file stream is invalid. Nothing will be written.");
		return;
	}

	for (uint_fast16_t i = 0; i < num_indent; i++) {
		fprintf(output, "\t");
	}
}

/****************************************************************************
* fprint_indent
*
* - prints the specified number of tabs to the specified file stream
* - used as indents to indicate the level of recursion in the output files
* of loud runs
*
* Parameters :
* - output : the file stream to print the tabs to
* - num_indent : the number of tabs to print
*
* Returns :
* - none
****************************************************************************/
// move to Misc.h?
// trade off of cost from allocation vs. repeated calls to fprintf...will have to do some benchmarking
//void fprint_indent2(FILE* __restrict output, const uint_fast16_t num_indent)
//{
//	if (output == NULL) {
//		DISPLAY_ERR(true,
//			"The supplied file stream is invalid. Nothing will be written.");
//		return;
//	}
//	uint_fast16_t arb_thresh = 0;
//	if (num_indent <= arb_thresh) {
//		char buff[500 + 1] = {'\t'}; // some constant buffer size known at compile time, maybe make it a #define for transparency
//		buff[num_indent] = '\0';
//		fprintf(output, buff);
//	} else {
//		char* buff = (char*)malloc(num_indent + 1);
//		if (buff == NULL) { // if we failed to allocate just complete the writes the slow way...
//			for (uint_fast16_t i = 0; i < num_indent; i++) {
//				fprintf(output, "\t");
//			}
//		}
//		else {
//			memset(buff, (char)'\t', num_indent);
//			buff[num_indent] = '\0';
//			fprintf(output, buff);
//			free(buff);
//		}
//	}
//	// basic approach
//	for (uint_fast16_t i = 0; i < num_indent; i++) {
//		fprintf(output, "\t");
//	}
//}

/****************************************************************************
* progress_log
*
* - prints a message to the supplied file stream with the specified indent
* - used to more easily write to the output files of loud runs
* 
*
* Parameters :
* - output : the file stream to print the tabs to
* - num_indent : the number of tabs to print (typically the recursion level
* of the current call)
* - format : the "printf style" format string to be translated and printed 
* to the file
* - ... : variable number of optional arguments corresponding to the format 
* string
*
* Returns :
* - none
****************************************************************************/
void progress_log(FILE* __restrict output, const uint_fast16_t num_indent, const char* format, ...)
{
	if (output == NULL) {
		DISPLAY_ERR(true, "The supplied file stream is invalid. Nothing will be written.");
		return;
	}

	fprint_indent(output, num_indent);

	va_list argptr;
	va_start(argptr, format);
	vfprintf(output, format, argptr);
	va_end(argptr);
}

/****************************************************************************
* fprint_move_hist
*
* - prints the current move history to the specified file stream
* - used to more easily write move history information to the output files 
* of loud runs
*
*
* Parameters :
* - output : the file stream to print the tabs to
* - recur_depth : typically the recursion level of the current call, used to
* specify how many moves to access out of the move_hist memory block
* - move_hist : block of memory holding the current move history of the game
*
* Returns :
* - none
****************************************************************************/
void fprint_move_hist(FILE* __restrict output, const uint_fast16_t recur_depth, std::vector<uint_fast16_t>& __restrict move_hist)
{
	if (output == NULL) {
		DISPLAY_ERR(true, "The supplied file stream is invalid. Nothing will be written.");
		return;
	}

	fprintf(output, "%hu", (uint16_t)move_hist[0]); // making the assumption there's at least one entry
	for (uint_fast16_t i = 1; i <= recur_depth; i++) {
		fprintf(output, "->%hu", (uint16_t)move_hist[i]); // added cast to provide consistent operation, regardless of what the compiler decided for uint_fast16_tss
	}
}

/****************************************************************************
* play_MAC_quiet
*
* - Plays the MAC game on the specified graph by recursively attempting to 
* find a winning move from each player's "perspective" in turn
*
* Parameters :
* - curr node : the current node in the MAC game, as designated by the ordering
* given in the graph's adjacency matrix
* - num_nodes : the number of nodes in the graph
* - adj_matrix : reference to a vector holding the adjency matrix for 
* the graph in question
* - edge_use_matrix : reference to a vector keeping track of which edges
* have been used so far in the game
* - node_use_list : reference to a vector keeping track of which nodes have
* been used so far in the game
*
* Returns :
* - GAME_STATE : indication of whether the game is in a WIN_STATE or LOSS_STATE
****************************************************************************/
GAME_STATE play_MAC_quiet(const uint_fast16_t curr_node, const uint_fast16_t num_nodes, const std::vector<uint_fast16_t>& __restrict adj_matrix,
	std::vector<uint_fast16_t>& __restrict edge_use_matrix, std::vector<uint_fast16_t>& __restrict node_use_list)
{
	uint_fast16_t open_edges = 0; // stores the number of available edges we can move along from curr_node
	GAME_STATE move_result; // temporarily store the result of a recursive call here

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++) {
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == NOT_USED) { // and the edge between them is unused
			open_edges++;
			if (node_use_list[curr_neighbor] == USED) { // if the neighbor has been previously visited, going back creates a cycle!
				return WIN_STATE;
			}
		}
	}

	if (open_edges == 0) { // if there are 0 open edges, we're in a loss state
		return LOSS_STATE;
	}

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++) {
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == NOT_USED) { // and the edge between them is unused
			// try making the move along that edge
			edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = USED;
			edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = USED; // have to mark both entries
			node_use_list[curr_neighbor] = USED;
			move_result = play_MAC_quiet(curr_neighbor, num_nodes, adj_matrix, edge_use_matrix, node_use_list);
			// reset the move after returning
			edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = NOT_USED;
			edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = NOT_USED; // have to mark both entries
			node_use_list[curr_neighbor] = NOT_USED;
			if (move_result == LOSS_STATE) { // if the move puts the game into a loss state, then the current state is a win state
				return WIN_STATE;
			}
			if (move_result == ERROR_STATE) {
				return ERROR_STATE;
			}
		}
	}

	// if we've gotten to this point there's no good moves-> game is in a loss state
	return LOSS_STATE; 
}

/****************************************************************************
* play_MAC_loud
*
* - Plays the MAC game on the specified graph by recursively attempting to
* find a winning move from each player's "perspective" in turn
* - The "loud" version prints its progress in playing through the game to a
* file as it goes
*
* Parameters :
* - curr node : the current node in the MAC game, as designated by the ordering
* given in the graph's adjacency matrix
* - num_nodes : the number of nodes in the graph
* - adj_matrix : reference to a vector holding the adjency matrix for
* the graph in question
* - edge_use_matrix : reference to a vector keeping track of which edges
* have been used so far in the game
* - node_use_list : reference to a vector keeping track of which nodes have
* been used so far in the game
* - move_hist : reference to a vector keeping track of the current chain's 
* move history
* - recur_depth : the recursive depth of the current call
* - output : the file to output our progress to
*
* Returns :
* - GAME_STATE : indication of whether the game is in a WIN_STATE or LOSS_STATE
****************************************************************************/
GAME_STATE play_MAC_loud(const uint_fast16_t curr_node, const uint_fast16_t num_nodes, const std::vector<uint_fast16_t>& __restrict adj_matrix,
	std::vector<uint_fast16_t>& __restrict edge_use_matrix, std::vector<uint_fast16_t>& __restrict node_use_list, std::vector<uint_fast16_t>& __restrict move_hist,
	const uint_fast16_t recur_depth, FILE* __restrict output)
{
	uint_fast16_t open_edges = 0; // stores the number of available edges we can move along from curr_node
	GAME_STATE move_result; // temporarily store the result of a recursive call here
	move_hist[recur_depth] = curr_node; // record the current position in the move history

	progress_log(output, recur_depth, "%s Reached node %hu\n", recur_depth % 2 == 0 ? "P1:" : "P2:", (uint16_t)curr_node);

	progress_log(output, recur_depth, "Checking for any cycles that are one move away.\n");
	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++) {
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == NOT_USED) { // and the edge between them is unused
			progress_log(output, recur_depth, "%s Checking the play from node %hu to %hu\n", 
				recur_depth % 2 == 0 ? "P1:" : "P2:", (uint16_t)curr_node, (uint16_t)curr_neighbor);
			open_edges++;
			if (node_use_list[curr_neighbor] == USED) { // if the neighbor has been previously visited, going back creates a cycle!
				progress_log(output, recur_depth, "Cycle detected. Move history: ");
				fprint_move_hist(output, recur_depth, move_hist);
				fprintf(output, "->%hu\n", (uint16_t)curr_neighbor); // since we don't formally "move" to this node, it's not included in the move_hist array
				return WIN_STATE;
			}
			progress_log(output, recur_depth, "%s No cycle detected\n", recur_depth % 2 == 0 ? "P1:" : "P2:");
		}
	}

	if (open_edges == 0) { // if there are 0 open edges, we're in a loss state
		progress_log(output, recur_depth, "No valid moves remaining.\n");
		progress_log(output, recur_depth, "Move History: ");
		fprint_move_hist(output, recur_depth, move_hist);
		progress_log(output, recur_depth, "\n");
		return LOSS_STATE;
	}

	progress_log(output, recur_depth, "Checking all available moves now.\n");
	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++) {
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == NOT_USED) { // and the edge between them is unused
			// try making the move along that edge
			edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = USED;
			edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = USED; // have to mark both entries 
			node_use_list[curr_neighbor] = USED;
			move_result = play_MAC_loud(curr_neighbor, num_nodes, adj_matrix, edge_use_matrix, node_use_list, move_hist, recur_depth + 1, output);
			// reset the move after returning
			edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = NOT_USED;
			edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = NOT_USED; // have to mark both entries 
			node_use_list[curr_neighbor] = NOT_USED;
			progress_log(output, recur_depth, "%s Playing from %hu to %hu results in a %s.",
				recur_depth % 2 == 0 ? "P1:" : "P2:", (uint16_t)curr_node, (uint16_t)curr_neighbor, move_result == WIN_STATE ? "LOSS_STATE" : "WIN_STATE");
			progress_log(output, 0, "Move history: "); // haven't gone to a new line yet so we set recursion depth to 0
			fprint_move_hist(output, recur_depth, move_hist);
			fprintf(output, "->%hu\n", (uint16_t)curr_neighbor); // since we don't formally "move" to this node, it's not included in the move_hist array
			if (move_result == LOSS_STATE) { // if the move puts the game into a loss state, then the current state is a win state
				return WIN_STATE;
			}
			if (move_result == ERROR_STATE) {
				return ERROR_STATE;
			}
		}
	}

	// if we've gotten to this point there's no good moves-> game is in a loss state
	progress_log(output, recur_depth, "%s No good moves, the game is in a loss state.\n", 
		recur_depth % 2 == 0 ? "P1:" : "P2:", curr_node);
	return LOSS_STATE;
}

/****************************************************************************
* play_AAC_quiet
*
* - Plays the AAC game on the specified graph by recursively attempting to
* find a winning move from each player's "perspective" in turn
*
* Parameters :
* - curr node : the current node in the MAC game, as designated by the ordering
* given in the graph's adjacency matrix
* - num_nodes : the number of nodes in the graph
* - adj_matrix : reference to a vector holding the adjency matrix for
* the graph in question
* - edge_use_matrix : reference to a vector keeping track of which edges
* have been used so far in the game
* - node_use_list : reference to a vector keeping track of which nodes have
* been used so far in the game
*
* Returns :
* - GAME_STATE : indication of whether the game is in a WIN_STATE or LOSS_STATE
****************************************************************************/
GAME_STATE play_AAC_quiet(const uint_fast16_t curr_node, const uint_fast16_t num_nodes, const std::vector<uint_fast16_t>& __restrict adj_matrix,
	std::vector<uint_fast16_t>& __restrict edge_use_matrix, std::vector<uint_fast16_t>& __restrict node_use_list)
{	
	GAME_STATE move_result; // temporarily store the result of a recursive call here

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++) {
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == NOT_USED) { // and the edge between them is unused
			if (node_use_list[curr_neighbor] == NOT_USED) { // move doesn't immediately result in a cycle, might as well try it out
				// try making the move along that edge
				edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = USED;
				edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = USED; // have to mark both entries 
				node_use_list[curr_neighbor] = USED;
				move_result = play_AAC_quiet(curr_neighbor, num_nodes, adj_matrix, edge_use_matrix, node_use_list);
				// reset the move after returning
				edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = NOT_USED;
				edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = NOT_USED; // have to mark both entries 
				node_use_list[curr_neighbor] = NOT_USED;
				if (move_result == LOSS_STATE) { // if the move puts the game into a loss state, then the current state is a win state
					return WIN_STATE;
				}
				if (move_result == ERROR_STATE) {
					return ERROR_STATE;
				}
			}
		}
	}

	// if we've gotten to this point there's no good moves-> game is in a loss state
	return LOSS_STATE;
}

/****************************************************************************
* play_AAC_loud
*
* - Plays the AAC game on the specified graph by recursively attempting to
* find a winning move from each player's "perspective" in turn
*
* Parameters :
* - curr node : the current node in the MAC game, as designated by the ordering
* given in the graph's adjacency matrix
* - num_nodes : the number of nodes in the graph
* - adj_matrix : reference to a vector holding the adjency matrix for
* the graph in question
* - edge_use_matrix : reference to a vector keeping track of which edges
* have been used so far in the game
* - node_use_list : reference to a vector keeping track of which nodes have
* been used so far in the game
*
* Returns :
* - GAME_STATE : indication of whether the game is in a WIN_STATE or LOSS_STATE
****************************************************************************/
GAME_STATE play_AAC_loud(const uint_fast16_t curr_node, const uint_fast16_t num_nodes, const std::vector<uint_fast16_t>& __restrict adj_matrix,
	std::vector<uint_fast16_t>& __restrict edge_use_matrix, std::vector<uint_fast16_t>& __restrict node_use_list, std::vector<uint_fast16_t>& __restrict move_hist,
	const uint_fast16_t recur_depth, FILE* __restrict output)
{	
	GAME_STATE move_result; // temporarily store the result of a recursive call here
	move_hist[recur_depth] = curr_node; // record the current position in the move history

	progress_log(output, recur_depth, "%s Reached node %hu\n", recur_depth % 2 == 0 ? "P1:" : "P2:", (uint16_t)curr_node);

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++) {
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == NOT_USED) { // and the edge between them is unused
			if (node_use_list[curr_neighbor] == NOT_USED) { // move doesn't immediately result in a cycle, might as well try it out
				progress_log(output, recur_depth, "%s Checking the play from node %hu to %hu\n",
					recur_depth % 2 == 0 ? "P1:" : "P2:", (uint16_t)curr_node, (uint16_t)curr_neighbor);
				// try making the move along that edge
				edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = USED;
				edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = USED; // have to mark both entries 
				node_use_list[curr_neighbor] = USED;
				move_result = play_AAC_loud(curr_neighbor, num_nodes, adj_matrix, edge_use_matrix, node_use_list, move_hist, recur_depth + 1, output);
				// reset the move after returning
				edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = NOT_USED;
				edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = NOT_USED; // have to mark both entries 
				node_use_list[curr_neighbor] = NOT_USED;
				progress_log(output, recur_depth, "%s Playing from %hu to %hu results in a %s. ",
					recur_depth % 2 == 0 ? "P1:" : "P2:", (uint16_t)curr_node, (uint16_t)curr_neighbor, move_result == WIN_STATE ? "LOSS_STATE" : "WIN_STATE");
				progress_log(output, 0, "Move history: ");
				fprint_move_hist(output, recur_depth, move_hist);
				fprintf(output, "->%hu\n", (uint16_t)curr_neighbor); // since we don't formally "move" to this node, it's not included in the move_hist array
				if (move_result == LOSS_STATE) { // if the move puts the game into a loss state, then the current state is a win state
					return WIN_STATE;
				}
				if (move_result == ERROR_STATE) {
					return ERROR_STATE;
				}
			}
		}
	}

	// if we've gotten to this point there's no good moves-> game is in a loss state
	progress_log(output, recur_depth, "%s No good moves, the game is in a loss state.\n",
		recur_depth % 2 == 0 ? "P1:" : "P2:", curr_node);
	return LOSS_STATE;
}

