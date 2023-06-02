#pragma once
#include "Cycle_Games.h"
#include <thread>

// current plan:
	// implement dispatch and normal threaded functions
	// add thread pool to project-> use jthreads to take advantage of stop_token's
		// do we want to add templated arguments? defining a bunch of structs and casting pointers is bad practice...
	// once implemented, need to do some testing to see where the trade off of the thread overhead is worth/ not worth
	// would need to look at number of available cores on system, and degree of the starting node in the graph
	// top level dispatch only, or allow arbitrary uses of threads from pool?

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
GAME_STATE play_MAC_threaded(const uint_fast16_t curr_node, const uint_fast16_t num_nodes, const std::vector<uint_fast16_t>& __restrict adj_matrix,
	std::vector<uint_fast16_t>& __restrict edge_use_matrix, std::vector<uint_fast16_t>& __restrict node_use_list)
{
	uint_fast16_t open_edges = 0; // stores the number of available edges we can move along from curr_node
	GAME_STATE move_result; // temporarily store the result of a recursive call here

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes; curr_neighbor++) {
		if (adj_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == ADJACENT // if curr_node and curr_neighbor are adjacent
			&& edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] == NOT_USED) { // and the edge between them is unused
			open_edges++;
			if (node_use_list[curr_neighbor] == USED) { // if the neighbor has been previously visited, going back creates a cycle!
				return GAME_STATE::WIN_STATE;
			}
		}
	}

	if (open_edges == 0) { // if there are 0 open edges, we're in a loss state
		return GAME_STATE::LOSS_STATE;
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
			if (move_result == GAME_STATE::LOSS_STATE) { // if the move puts the game into a loss state, then the current state is a win state
				return GAME_STATE::WIN_STATE;
			}
		}
	}

	// if we've gotten to this point there's no good moves-> game is in a loss state
	return GAME_STATE::LOSS_STATE;
}