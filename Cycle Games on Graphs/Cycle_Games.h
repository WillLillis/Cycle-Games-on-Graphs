/*
*
* - This file will serve as the "work horse" for our game playing code
* - In this header we will define the functions that take in and play the 
* MAC and AAC games, given a common input form (pointer to an array)
* 
* - Going to begin by copying over a bunch of my crap code from the first 
* pass on this project, and then cleaning it up
* 
* - We could make this an object oriented project with a bunch of classes and whatnot, 
* but that's not something I'm not super comfortable with, and also seems like it would 
* be an additional hurdle to future research interns looking to contribute to the code base
* 
* As a result of this (potentially misguided) decision, the code is going to rewritten 
* in a loose style, and hopefully that's ok
* 
* I'm a bit torn in regards to using the std::vector class or just raw arrays
*	- As some of these runs take quite a bit of time, any amount of overhead we can
*	drop is helpful
*	- going to go with raw arrays for now, but I may end up backtracking on this for
*	std::vector's ease of use
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

// We'll define the game state in the following manner
typedef uint_fast8_t GAME_STATE;
#define WIN_STATE		1
#define LOSS_STATE		0

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


void fprint_indent(FILE* output, uint_fast16_t num_indent)
{
	assert(output != NULL);

	for (uint_fast16_t i = 0; i < num_indent; i++)
	{
		fprintf(output, "\t");
	}
}

void progress_log(FILE* output, uint_fast16_t num_indent, const char* format, ...)
{
	assert(output != NULL);

	fprint_indent(output, num_indent);

	va_list argptr;

	va_start(argptr, format);
	vfprintf(output, format, argptr);
	va_end(argptr);
}

void fprint_move_hist(FILE* output, uint_fast16_t recur_depth, uint_fast16_t* move_hist)
{
	assert(output != NULL);

	fprintf(output, "%hhu", move_hist[0]); // making the assumption there's at least one entry
	for (uint_fast16_t i = 0; i <= recur_depth; i++)
	{
		fprintf(output, "->%hhu", move_hist[i]);
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
* - adj_matrix : pointer to an block of memory holding the adjency matrix for 
* the graph in question
* - edge_use_matrix : pointer to an block of memory keeping track of which edges
* have been used so far in the game
* - node_use_list : pointer to a block of memory keeping track of which nodes have
* been used so far in the game
*
* Returns :
* - GAME_STATE : indication of whether the game is in a WIN_STATE or LOSS_STATE
****************************************************************************/
GAME_STATE play_MAC_quiet(uint_fast16_t curr_node, uint_fast16_t num_nodes, uint_fast16_t* adj_matrix,
	uint_fast16_t* edge_use_matrix, uint_fast16_t* node_use_list)
{
	// need to account for marking the starting node as used
	node_use_list[curr_node] = USED;
	uint_fast16_t open_edges = 0; // stores the number of available edges we can move along from curr_node
	GAME_STATE move_result = 0; // temporarily store the result of a recursive call here

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++)
	{
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == NOT_USED) // and the edge between them is unused
		{
			open_edges++;
			if (node_use_list[curr_neighbor] == USED) // if the neighbor has been previously visited, going back creates a cycle!
			{
				return WIN_STATE;
			}
		}
	}

	if (open_edges == 0) // if there are 0 open edges, we're in a loss state
	{
		return LOSS_STATE;
	}

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++)
	{
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == NOT_USED) // and the edge between them is unused
		{
			// try making the move along that edge
			edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = USED;
			edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = USED; // have to mark both entries
			node_use_list[curr_neighbor] = USED;
			move_result = play_MAC_quiet(curr_neighbor, num_nodes, adj_matrix, edge_use_matrix, node_use_list);
			// reset the move after returning
			edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = NOT_USED;
			edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = NOT_USED; // have to mark both entries
			node_use_list[curr_neighbor] = NOT_USED;
			if (move_result == LOSS_STATE) // if the move puts the game into a loss state, then the current state is a win state
			{
				return WIN_STATE;
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
* - adj_matrix : pointer to an block of memory holding the adjency matrix for
* the graph in question
* - edge_use_matrix : pointer to an block of memory keeping track of which edges
* have been used so far in the game
* - node_use_list : pointer to a block of memory keeping track of which nodes have
* been used so far in the game
* - move_hist : pointer to a block of memory keeping track of the current chain's 
* move history
* - recur_depth : the recursive depth of the current call
* - output : the file to output our progress to
*
* Returns :
* - GAME_STATE : indication of whether the game is in a WIN_STATE or LOSS_STATE
****************************************************************************/
GAME_STATE play_MAC_loud(uint_fast16_t curr_node, uint_fast16_t num_nodes, uint_fast16_t* adj_matrix,
	uint_fast16_t* edge_use_matrix, uint_fast16_t* node_use_list, uint_fast16_t* move_hist, uint_fast16_t recur_depth, FILE* output)
{
	assert(adj_matrix != NULL);
	// need to account for marking the starting node as used
	node_use_list[curr_node] = USED;

	uint_fast16_t open_edges = 0; // stores the number of available edges we can move along from curr_node
	GAME_STATE move_result = 0; // temporarily store the result of a recursive call here
	move_hist[recur_depth] = curr_node; // record the current position in the move history

	progress_log(output, recur_depth, "%s Reached node %hhu\n", recur_depth % 2 == 0 ? "P1:" : "P2:", curr_node);

	progress_log(output, recur_depth, "Checking for any cycles that are one move away.\n");
	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++)
	{
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == NOT_USED) // and the edge between them is unused
		{
			progress_log(output, recur_depth, "%s Checking the play from node %hhu to %hhu\n", 
				recur_depth % 2 == 0 ? "P1:" : "P2:", curr_node, curr_neighbor);
			open_edges++;
			if (node_use_list[curr_neighbor] == USED) // if the neighbor has been previously visited, going back creates a cycle!
			{
				progress_log(output, recur_depth, "Cycle detected. Move history: ");
				fprint_move_hist(output, recur_depth, move_hist);
				fprintf(output, "->%hhu\n", curr_neighbor); // since we don't formally "move" to this node, it's not included in the move_hist array
				return WIN_STATE;
			}
			progress_log(output, recur_depth, "%s No cycle detected\n", recur_depth % 2 == 0 ? "P1:" : "P2:");
		}
	}

	if (open_edges == 0) // if there are 0 open edges, we're in a loss state
	{
		progress_log(output, recur_depth, "No valid moves remaining.\n");
		progress_log(output, recur_depth, "Move History: ");
		fprint_move_hist(output, recur_depth, move_hist);
		progress_log(output, recur_depth, "\n");
		return LOSS_STATE;
	}

	progress_log(output, recur_depth, "Checking all available moves now.\n");
	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++)
	{
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == NOT_USED) // and the edge between them is unused
		{
			// try making the move along that edge
			edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = USED;
			edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = USED; // have to mark both entries 
			node_use_list[curr_neighbor] = USED;
			move_result = play_MAC_loud(curr_neighbor, num_nodes, adj_matrix, edge_use_matrix, node_use_list, move_hist, recur_depth + 1, output);
			// reset the move after returning
			edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = NOT_USED;
			edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = NOT_USED; // have to mark both entries 
			node_use_list[curr_neighbor] = NOT_USED;
			progress_log(output, recur_depth, "%s Playing from %hhu to %hhu results in a %s. ",
				recur_depth % 2 == 0 ? "P1:" : "P2:", curr_node, curr_neighbor, move_result == WIN_STATE ? "WIN_STATE" : "LOSS_STATE");
			progress_log(output, 0, "Move history: "); // haven't gone to a new line yet so we set recursion depth to 0
			fprint_move_hist(output, 0, move_hist);
			fprintf(output, "->%hhu\n", curr_neighbor); // since we don't formally "move" to this node, it's not included in the move_hist array
			if (move_result == LOSS_STATE) // if the move puts the game into a loss state, then the current state is a win state
			{
				return WIN_STATE;
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
* - adj_matrix : pointer to an block of memory holding the adjency matrix for
* the graph in question
* - edge_use_matrix : pointer to an block of memory keeping track of which edges
* have been used so far in the game
* - node_use_list : pointer to a block of memory keeping track of which nodes have
* been used so far in the game
*
* Returns :
* - GAME_STATE : indication of whether the game is in a WIN_STATE or LOSS_STATE
****************************************************************************/
GAME_STATE play_AAC_quiet(uint_fast16_t curr_node, uint_fast16_t num_nodes, uint_fast16_t* adj_matrix,
	uint_fast16_t* edge_use_matrix, uint_fast16_t* node_use_list)
{
	// need to account for marking the starting node as used
	node_use_list[curr_node] = USED;
	GAME_STATE move_result = 0; // temporarily store the result of a recursive call here

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++)
	{
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == NOT_USED) // and the edge between them is unused
		{
			if (node_use_list[curr_neighbor] == NOT_USED) // move doesn't immediately result in a cycle, might as well try it out
			{
				// try making the move along that edge
				edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = USED;
				edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = USED; // have to mark both entries 
				node_use_list[curr_neighbor] = USED;
				move_result = play_AAC_quiet(curr_neighbor, num_nodes, adj_matrix, edge_use_matrix, node_use_list);
				// reset the move after returning
				edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = NOT_USED;
				edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = NOT_USED; // have to mark both entries 
				node_use_list[curr_neighbor] = NOT_USED;
				if (move_result == LOSS_STATE) // if the move puts the game into a loss state, then the current state is a win state
				{
					return WIN_STATE;
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
* - adj_matrix : pointer to an block of memory holding the adjency matrix for
* the graph in question
* - edge_use_matrix : pointer to an block of memory keeping track of which edges
* have been used so far in the game
* - node_use_list : pointer to a block of memory keeping track of which nodes have
* been used so far in the game
*
* Returns :
* - GAME_STATE : indication of whether the game is in a WIN_STATE or LOSS_STATE
****************************************************************************/
GAME_STATE play_AAC_loud(uint_fast16_t curr_node, uint_fast16_t num_nodes, uint_fast16_t* adj_matrix,
	uint_fast16_t* edge_use_matrix, uint_fast16_t* node_use_list, uint_fast16_t* move_hist, uint_fast16_t recur_depth, FILE* output)
{
	// need to account for marking the starting node as used
	node_use_list[curr_node] = USED;
	GAME_STATE move_result = 0; // temporarily store the result of a recursive call here
	move_hist[recur_depth] = curr_node; // record the current position in the move history

	progress_log(output, recur_depth, "%s Reached node %hhu\n", recur_depth % 2 == 0 ? "P1:" : "P2:", curr_node);

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++)
	{
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == NOT_USED) // and the edge between them is unused
		{
			if (node_use_list[curr_neighbor] == NOT_USED) // move doesn't immediately result in a cycle, might as well try it out
			{
				progress_log(output, recur_depth, "%s Checking the play from node %hhu to %hhu\n",
					recur_depth % 2 == 0 ? "P1:" : "P2:", curr_node, curr_neighbor);
				// try making the move along that edge
				edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = USED;
				edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = USED; // have to mark both entries 
				node_use_list[curr_neighbor] = USED;
				move_result = play_AAC_loud(curr_neighbor, num_nodes, adj_matrix, edge_use_matrix, node_use_list, move_hist, recur_depth + 1, output);
				// reset the move after returning
				edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = NOT_USED;
				edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = NOT_USED; // have to mark both entries 
				node_use_list[curr_neighbor] = NOT_USED;
				progress_log(output, recur_depth, "%s Playing from %hhu to %hhu results in a %s. ",
					recur_depth % 2 == 0 ? "P1:" : "P2:", curr_node, curr_neighbor, move_result == WIN_STATE ? "WIN_STATE" : "LOSS_STATE");
				progress_log(output, recur_depth, "Move history: ");
				fprint_move_hist(output, recur_depth, move_hist);
				fprintf(output, "->%hhu\n", curr_neighbor); // since we don't formally "move" to this node, it's not included in the move_hist array
				if (move_result == LOSS_STATE) // if the move puts the game into a loss state, then the current state is a win state
				{
					return WIN_STATE;
				}
			}
		}
	}

	// if we've gotten to this point there's no good moves-> game is in a loss state
	progress_log(output, recur_depth, "%s No good moves, the game is in a loss state.\n",
		recur_depth % 2 == 0 ? "P1:" : "P2:", curr_node);
	return LOSS_STATE;
}