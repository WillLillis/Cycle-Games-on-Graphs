#pragma once
#include "Cycle_Games.h"
#include "Threadpool.h"

/*
* - So as far as I can tell the code below "works", but isn't really usable...
* - Each separate job needs its own private copy of the node_use and edge_use vectors
* and creating those copies gets very expensive and we typically run out of memory
* 
* - For graphs small enough so that we don't bottom out the stack, the threading overhead is > 10x
* - For graphs where threading would help, we crash because of the afforementioned memory issues
* 
* - Might be worth looking into heap-allocating the vectors and seeing if that works, but I'll try that later...
*		- looking at the docs vectors may already be heap allocated, so I don't think this is a solution
* 
* - I'll leave this code in case I think of a solution later...
* 
*/

GAME_STATE play_MAC_recur(const uint_fast16_t curr_node, const uint_fast16_t num_nodes, const std::vector<Adjacency_Info>& __restrict adj_matrix,
	std::vector<EDGE_STATE>& __restrict edge_use_matrix, std::vector<NODE_STATE>& __restrict node_use_list, std::stop_source token_source)
{
	std::stop_token stoken = token_source.get_token();
	if (stoken.stop_requested()) {
		return GAME_STATE::KILL_STATE;
	}

	uint_fast16_t open_edges = 0; // stores the number of available edges we can move along from curr_node
	GAME_STATE move_result; // temporarily store the result of a recursive call here

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++) {
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == Adjacency_Info::ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == EDGE_STATE::NOT_USED) { // and the edge between them is unused
			open_edges++;
			if (node_use_list[curr_neighbor] == NODE_STATE::USED) { // if the neighbor has been previously visited, going back creates a cycle!
				return GAME_STATE::WIN_STATE;
			}
		}
	}

	if (open_edges == 0) { // if there are 0 open edges, we're in a loss state
		return GAME_STATE::LOSS_STATE;
	}

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++) {
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == Adjacency_Info::ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == EDGE_STATE::NOT_USED) { // and the edge between them is unused
			// try making the move along that edge
			MARK_EDGE(edge_use_matrix, num_nodes, curr_neighbor, curr_node, EDGE_STATE::USED);
			node_use_list[curr_neighbor] = NODE_STATE::USED;
			move_result = play_MAC_recur(curr_neighbor, num_nodes, adj_matrix, edge_use_matrix, node_use_list, token_source);
			// reset the move after returning
			MARK_EDGE(edge_use_matrix, num_nodes, curr_neighbor, curr_node, EDGE_STATE::NOT_USED);
			node_use_list[curr_neighbor] = NODE_STATE::NOT_USED;
			if (move_result == GAME_STATE::LOSS_STATE) { // if the move puts the game into a loss state, then the current state is a win state
				return GAME_STATE::WIN_STATE;
			} else if(move_result == GAME_STATE::KILL_STATE) {
				return GAME_STATE::KILL_STATE;
			}
		}
	}

	// if we've gotten to this point there's no good moves-> game is in a loss state
	return GAME_STATE::LOSS_STATE;
}

/****************************************************************************
* play_MAC_threaded
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
GAME_STATE play_MAC_threaded(const uint_fast16_t curr_node, const uint_fast16_t num_nodes, const std::vector<Adjacency_Info>& __restrict adj_matrix,
	std::vector<EDGE_STATE>& __restrict edge_use_matrix, std::vector<NODE_STATE>& __restrict node_use_list)
{
	uint_fast16_t open_edges = 0; // stores the number of available edges we can move along from curr_node
	GAME_STATE game_result = GAME_STATE::LOSS_STATE; 

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++) {
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == Adjacency_Info::ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == EDGE_STATE::NOT_USED) { // and the edge between them is unused
			open_edges++;
			if (node_use_list[curr_neighbor] == NODE_STATE::USED) { // if the neighbor has been previously visited, going back creates a cycle!
				return GAME_STATE::WIN_STATE;
			}
		}
	}

	if (open_edges == 0) { // if there are 0 open edges, we're in a loss state
		return GAME_STATE::LOSS_STATE;
	}

	ThreadPool pool(3);
	std::stop_source token_source;
	std::stop_token stoken = token_source.get_token();
	std::vector<std::vector<EDGE_STATE>> edge_states;
	std::vector<std::vector<NODE_STATE>> node_states;
	std::vector<std::future<GAME_STATE>> returns;

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++) {
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == Adjacency_Info::ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == EDGE_STATE::NOT_USED) { // and the edge between them is unused
			edge_states.emplace_back(edge_use_matrix);
			node_states.emplace_back(node_use_list);
			MARK_EDGE(edge_states.back(), num_nodes, curr_neighbor, curr_node, EDGE_STATE::USED);
			returns.emplace_back(pool.enqueue(play_MAC_recur, curr_neighbor, num_nodes, adj_matrix, 
				std::ref(edge_states.back()), std::ref(node_states.back()), token_source));
		}
	}

	// now that we've queued up all the jobs, time to poll to see if there's any winners
	bool keep_checking = true; // indicates whether we're still waiting on at least one thread's return value
	while (keep_checking) {
		keep_checking = false;
		for (auto& ret_val : returns) {
			if (ret_val.valid()) {
				keep_checking = true;
				if (ret_val.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
					if (ret_val.get() == GAME_STATE::WIN_STATE) {
						game_result = GAME_STATE::WIN_STATE;
						token_source.request_stop();
						keep_checking = false;
						break;
					}
				}
			}
		}
	}
	
	// no need for explicit cleanup, the Threadpool destructor will call join() on all of the workers for us!
	return game_result;
}

GAME_STATE play_AAC_recur(const uint_fast16_t curr_node, const uint_fast16_t num_nodes, const std::vector<Adjacency_Info>& __restrict adj_matrix,
	std::vector<EDGE_STATE>& __restrict edge_use_matrix, std::vector<NODE_STATE>& __restrict node_use_list, std::stop_source token_source)
{
	std::stop_token stoken = token_source.get_token();
	if (stoken.stop_requested()) {
		return GAME_STATE::KILL_STATE;
	}

	GAME_STATE move_result; // temporarily store the result of a recursive call here

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++) {
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == Adjacency_Info::ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == EDGE_STATE::NOT_USED// and the edge between them is unused
			&& node_use_list[curr_neighbor] == NODE_STATE::NOT_USED) { // move doesn't immediately result in a cycle, might as well try it out
			// try making the move along that edge
			MARK_EDGE(edge_use_matrix, num_nodes, curr_neighbor, curr_node, EDGE_STATE::USED);
			node_use_list[curr_neighbor] = NODE_STATE::USED;
			move_result = play_AAC_quiet(curr_neighbor, num_nodes, adj_matrix, edge_use_matrix, node_use_list);
			// reset the move after returning
			MARK_EDGE(edge_use_matrix, num_nodes, curr_neighbor, curr_node, EDGE_STATE::NOT_USED);
			node_use_list[curr_neighbor] = NODE_STATE::NOT_USED;
			if (move_result == GAME_STATE::LOSS_STATE) { // if the move puts the game into a loss state, then the current state is a win state
				return GAME_STATE::WIN_STATE;
			} else if (move_result == GAME_STATE::KILL_STATE) {
				return GAME_STATE::KILL_STATE;
			}
		}
	}

	// if we've gotten to this point there's no good moves-> game is in a loss state
	return GAME_STATE::LOSS_STATE;
}

/****************************************************************************
* play_AAC_threaded
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
GAME_STATE play_AAC_threaded(const uint_fast16_t curr_node, const uint_fast16_t num_nodes, const std::vector<Adjacency_Info>& __restrict adj_matrix,
	std::vector<EDGE_STATE>& __restrict edge_use_matrix, std::vector<NODE_STATE>& __restrict node_use_list)
{
	GAME_STATE game_result = GAME_STATE::LOSS_STATE;

	ThreadPool pool(3);
	std::stop_source token_source;
	std::stop_token stoken = token_source.get_token();
	std::vector<std::vector<EDGE_STATE>> edge_states;
	std::vector<std::vector<NODE_STATE>> node_states;
	std::vector<std::future<GAME_STATE>> returns;

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++) {
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == Adjacency_Info::ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == EDGE_STATE::NOT_USED// and the edge between them is unused
			&& node_use_list[curr_neighbor] == NODE_STATE::NOT_USED) { // move doesn't immediately result in a cycle, might as well try it out
			// try making the move along that edge
			edge_states.emplace_back(edge_use_matrix);
			node_states.emplace_back(node_use_list);
			MARK_EDGE(edge_states.back(), num_nodes, curr_neighbor, curr_node, EDGE_STATE::USED);
			node_use_list[curr_neighbor] = NODE_STATE::USED;
			returns.emplace_back(pool.enqueue(play_AAC_recur, curr_neighbor, num_nodes, adj_matrix,
				std::ref(edge_states.back()), std::ref(node_states.back()), token_source));
		}
	}

	// now that we've queued up all the jobs, time to poll to see if there's any winners
	bool keep_checking = true; // indicates whether we're still waiting on at least one thread's return value
	while (keep_checking) {
		keep_checking = false;
		for (auto& ret_val : returns) {
			if (ret_val.valid()) {
				keep_checking = true;
				if (ret_val.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
					if (ret_val.get() == GAME_STATE::WIN_STATE) {
						game_result = GAME_STATE::WIN_STATE;
						token_source.request_stop();
						keep_checking = false;
						break;
					}
				}
			}
		}
	}

	// no need for explicit cleanup, the Threadpool destructor will call join() on all of the workers for us!
	return game_result;
}