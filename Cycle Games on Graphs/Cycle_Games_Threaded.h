#pragma once

#include "Cycle_Games.h"

// going to start drafting the multithreaded version here
// The way to do this was a lot less self evident to me as compared to the normal MAC/AAC play game functions
// There's definitely a better way to do this, but here's a first go at it
/*
*
* The basic algorthm is the same-> Just try out moves and see if we can place the other player in a loss state
* At the top level, the point in the function where moves are tried out will simply dispatch threads to try them out
*	- We are expressly avoiding recursively spawning threads, and instead are only creating and dispatching at the top level
* The function will have an array of structs. Each struct will hold its own
*	- edge use matrix
*	- node use matrix
*	- "return value" variable passed in by reference
*		- set to a default value at the start, so that the thread can signal to the top level when it's actually done
*	- a "kill flag"
*		- the c++ std::thread api doesn't have any kill commands. I know the windows thread api does, but we're making this
*		work on multiple platforms, so I don't think using that would be the best approach to the problem
*		- we'll just have to write the functions in a way so that they periodically check the kill flag, and return some
*		kind of "killed" status back up their call stack once recognizing it
* At the top level, we'll need to periodically check every thread that's been dispatched to see if it's exited yet
*	- this will be baked in as part of each pass of the recursive move for loop
*	- If a thread exits with a LOSS_STATE, then we're done-> Just need to set the kill flags for all the other active threas
*	and wait for them to join\
*	- If a thread exits with a WIN_STATE, we need to join it, reset the other members of its struct (memset 0 mostly), and then
*	mark the thread as available for dispatch
*	- If all the moves are tried, then we just wait for the remaining active threads to join. If any of them exit with a LOSS_STATE, we set
*	the kill flag for the others and wait for them to join
*	- If all they all exit with a WIN_STATE, we exit the main top level function with a LOSS_STATE
*
*/


// Might redo this later in a more modern C++ style, but wanted to try this out for now
// C++ structs need initializers...you learn something new everyday
typedef struct THREAD_GAME_INFO {
	std::thread thread{};
	uint_fast16_t* edge_use_matrix = NULL;
	uint_fast16_t* node_use_list = NULL;
	GAME_STATE* return_val = NULL;
	bool* kill_flag = NULL;
	bool avail_for_use = true;
}THREAD_GAME_INFO;

bool thread_game_info_init(THREAD_GAME_INFO* new_struct, uint_fast16_t num_nodes, uint_fast16_t starting_node)
{
	if (new_struct == NULL)
	{
		return false;
	}

	new_struct->edge_use_matrix = (uint_fast16_t*)calloc(num_nodes * num_nodes, sizeof(uint_fast16_t));
	new_struct->node_use_list = (uint_fast16_t*)calloc(num_nodes, sizeof(uint_fast16_t));
	new_struct->return_val = (GAME_STATE*)malloc(sizeof(GAME_STATE));
	new_struct->kill_flag = (bool*)malloc(sizeof(bool));
	//new_struct->avail_for_use = true; // taken care of with the struct's initializer

	if (new_struct->edge_use_matrix == NULL
		|| new_struct->node_use_list == NULL
		|| new_struct->return_val == NULL
		|| new_struct->kill_flag == NULL)
	{
		if (new_struct->edge_use_matrix != NULL)
		{
			free(new_struct->edge_use_matrix);
		}
		if (new_struct->node_use_list != NULL)
		{
			free(new_struct->node_use_list);
		}
		if (new_struct->return_val != NULL)
		{
			free(new_struct->return_val);
		}
		if (new_struct->kill_flag != NULL)
		{
			free(new_struct->kill_flag);
		}

		return false;
	}

	*(new_struct->kill_flag) = false;
	*(new_struct->return_val) = ERROR_STATE;
	new_struct->node_use_list[starting_node] = USED;
	return true;
}

void thread_game_info_free(THREAD_GAME_INFO* old_struct)
{
	if (old_struct == NULL)
	{
		return;
	}

	if (old_struct->edge_use_matrix != NULL)
	{
		free(old_struct->edge_use_matrix);
	}
	if (old_struct->node_use_list != NULL)
	{
		free(old_struct->node_use_list);
	}
	if (old_struct->return_val != NULL)
	{
		free(old_struct->return_val);
	}
	if (old_struct->kill_flag != NULL)
	{
		free(old_struct->kill_flag);
	}

	free(old_struct);
}

bool thread_game_info_reset(THREAD_GAME_INFO* old_struct, uint_fast16_t num_nodes, uint_fast16_t starting_node)
{
	if (old_struct == NULL
		|| old_struct->return_val == NULL
		|| old_struct->kill_flag == NULL)
	{
		return false;
	}

	/*
	* 
	* with how the MAC_threaded_rucur function operates, the edge_use_matrix and node_use_list parts should be unecessary...
	* however we'll leave them in the function, commented out just in case unexpected bugs arise
	* 
	if (old_struct->edge_use_matrix == NULL
		|| old_struct->node_use_list == NULL)
	{
		return false;
	}
	memset(old_struct->edge_use_matrix, (uint_fast16_t)NOT_USED, num_nodes * num_nodes * sizeof(uint_fast16_t));
	memset(old_struct->node_use_list, (uint_fast16_t)NOT_USED, num_nodes * sizeof(uint_fast16_t));
	old_struct->node_use_list[starting_node] = USED;
	*/

	*(old_struct->return_val) = ERROR_STATE;
	*(old_struct->kill_flag) = false;
	old_struct->avail_for_use = true;
	return true;
}

// will block by design
uint_fast16_t next_avail_thread(THREAD_GAME_INFO* thread_list, uint_fast16_t num_threads, uint_fast16_t starting_thread = 0)
{
	uint_fast16_t curr_thread = starting_thread;
	while (true)
	{
		if (thread_list[curr_thread].avail_for_use) // just reading, so shouldn't have to worry about mutexes and whatnot
		{
			return curr_thread;
		}
		curr_thread = (curr_thread + 1) % num_threads;
	}
}

// pretty much the quiet version of the regular game playing code, except we add in checks for the kill flag
GAME_STATE MAC_threaded_rucur(uint_fast16_t curr_node, uint_fast16_t num_nodes, uint_fast16_t* adj_matrix,
	uint_fast16_t* edge_use_matrix, uint_fast16_t* node_use_list, const volatile bool* kill_flag)
{
	if (adj_matrix == NULL || edge_use_matrix == NULL 
		|| node_use_list == NULL || kill_flag == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Invalid memory addresses passed to the function.");
		return ERROR_STATE;
	}
	if (*kill_flag)
	{
		return KILL_STATE;
	}

	uint_fast16_t open_edges = 0; // stores the number of available edges we can move along from curr_node
	GAME_STATE move_result; // temporarily store the result of a recursive call here

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
			move_result = MAC_threaded_rucur(curr_neighbor, num_nodes, adj_matrix, edge_use_matrix, node_use_list, kill_flag);
			// reset the move after returning
			edge_use_matrix[index_translation(num_nodes, curr_node, curr_neighbor)] = NOT_USED;
			edge_use_matrix[index_translation(num_nodes, curr_neighbor, curr_node)] = NOT_USED; // have to mark both entries
			node_use_list[curr_neighbor] = NOT_USED;
			if (move_result == LOSS_STATE) // if the move puts the game into a loss state, then the current state is a win state
			{
				return WIN_STATE;
			}
			if (move_result == ERROR_STATE)
			{
				return ERROR_STATE;
			}
		}
	}

	// if we've gotten to this point there's no good moves-> game is in a loss state
	return LOSS_STATE;
}

// function called directly from the top level
// used for calling the recursive game code, but then getting that result and sending it back up via the return_val param
void MAC_threaded_dispatch(uint_fast16_t starting_node, uint_fast16_t num_nodes, uint_fast16_t* adj_matrix, 
	THREAD_GAME_INFO* thread_materials)
{
	// simply call the recursive game playing code and store the result where the caller can see it
	*(thread_materials->return_val) = MAC_threaded_rucur(starting_node, num_nodes, adj_matrix, 
		thread_materials->edge_use_matrix, thread_materials->node_use_list, thread_materials->kill_flag);
}


GAME_STATE play_MAC_threaded(uint_fast16_t starting_node, uint_fast16_t num_nodes, uint_fast16_t* adj_matrix,
	uint_fast16_t* edge_use_matrix, uint_fast16_t* node_use_list)
{
	if (adj_matrix == NULL || edge_use_matrix == NULL || node_use_list == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Invalid memory addresses passed to the function.");
		return ERROR_STATE;
	}

	const uint_fast16_t num_threads = std::thread::hardware_concurrency();
	THREAD_GAME_INFO* avail_threads = (THREAD_GAME_INFO*)malloc(num_threads * sizeof(THREAD_GAME_INFO));

	if (avail_threads == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Memory allocation error! Requested %zu bytes for the \"avail_threads\" variable", num_threads * sizeof(THREAD_GAME_INFO));
		return ERROR_STATE;
	}
	// as long as the memory allocation worked, we need to initialize the structs' data members now...
	for (uint_fast16_t i = 0; i < num_threads; i++)
	{
		if (!thread_game_info_init(&avail_threads[i], num_nodes, starting_node)) // if there was an allocation error
		{
			for (uint_fast16_t j = 0; j <= i; j++) // free everything else up and return an ERROR_STATE
			{
				thread_game_info_free(&avail_threads[i]);
				return ERROR_STATE;
			}
		}
	}

	// if we got to this point everything succeeded, time to do some dispatching
	uint_fast16_t next_thread = 0; // just a suggestion
	uint_fast16_t curr_thread = 0;
	bool search_continue = true;
	GAME_STATE exit_reason = RUN_STATE; // the state we found recursively that caused us to exit the main move loop

	for (uint_fast16_t curr_neighbor = 0; curr_neighbor < num_nodes && search_continue; curr_neighbor++)
	{
		// if curr_node and curr_neighbor are adjacent
		if (adj_matrix[index_translation(num_nodes, starting_node, curr_neighbor)] == ADJACENT) // don't need to check edge usage since this is the top level
		{
			// try making the move along that edge
			edge_use_matrix[index_translation(num_nodes, starting_node, curr_neighbor)] = USED;
			edge_use_matrix[index_translation(num_nodes, curr_neighbor, starting_node)] = USED; // have to mark both entries
			node_use_list[curr_neighbor] = USED;

			// find the next available thread, and dispatch it to try out the move
			curr_thread = next_avail_thread(avail_threads, num_threads, next_thread++);
			avail_threads[curr_thread].avail_for_use = false;
			avail_threads[curr_thread].edge_use_matrix[index_translation(num_nodes, starting_node, curr_neighbor)] = USED;
			avail_threads[curr_thread].edge_use_matrix[index_translation(num_nodes, curr_neighbor, starting_node)] = USED;
			avail_threads[curr_thread].node_use_list[curr_neighbor] = USED;
			*(avail_threads[curr_thread].return_val) = RUN_STATE;
			avail_threads[curr_thread].thread = std::thread(MAC_threaded_dispatch, starting_node, num_nodes, adj_matrix, &avail_threads[curr_thread]);
		}

		// check and see if any of the other threads have finished yet
		for (uint_fast16_t i = 0; i < num_threads; i++)
		{
			if (avail_threads[i].avail_for_use == false) // if the thread is currently running...
			{
				if (*(avail_threads[i].return_val) == RUN_STATE) // if it's still running, let it be
				{
					continue;
				}
				else if (*(avail_threads[i].return_val) == ERROR_STATE) // if there was an error, it's time to end everything
				{
					// error handling code
					avail_threads[i].thread.join(); // should join immediately
					exit_reason = ERROR_STATE;
					search_continue = false;
					break;
				}
				else if (*(avail_threads[i].return_val) == WIN_STATE) // we don't care, clean the thread up and set it as ready for the next dispatch
				{
					avail_threads[i].thread.join(); // should join immediately
					if (!thread_game_info_reset(&avail_threads[i], num_nodes, starting_node)) // make sure the reset was successful
					{
						exit_reason = ERROR_STATE;
						search_continue = false;
						break;
					}
					continue;
				}
				else if (*(avail_threads[i].return_val) == LOSS_STATE) // a winning move, also time to end everything
				{
					avail_threads[i].thread.join(); // should join immediately
					exit_reason = LOSS_STATE;
					search_continue = false;
					break;
				}
			}
		}
	}

	GAME_STATE return_val = ERROR_STATE;

	if (exit_reason == RUN_STATE) // we still have active threads that we care about
	{
		uint_fast16_t threads_left = 0;
		search_continue = true;
		while (search_continue)
		{
			threads_left = 0;
			for (uint_fast16_t i = 0; i < num_threads; i++) // for all of our threads...
			{
				// false positive warning (C6385) from VS about reading invalid data
				#pragma warning(suppress:6385)
				if (avail_threads[i].avail_for_use == false) // if the thread is currently running...
				{
					threads_left++;
					if (*(avail_threads[i].return_val) == RUN_STATE) // if it's still running, let it be
					{
						continue;
					}
					else if (*(avail_threads[i].return_val) == ERROR_STATE) // if there was an error, it's time to end everything
					{
						avail_threads[i].thread.join(); // should join immediately
						exit_reason = ERROR_STATE;
						search_continue = false;
						break;
					}
					else if (*(avail_threads[i].return_val) == WIN_STATE) // we don't care, reset the struct (can't free it since we're still checking its value on the next pass)
					{				
						avail_threads[i].thread.join(); // should join immediately
						if (!thread_game_info_reset(&avail_threads[i], num_nodes, starting_node)) // make sure the reset was successful
						{
							exit_reason = ERROR_STATE;
							search_continue = false;
							break;
						}
						continue;
					}
					else if (*(avail_threads[i].return_val) == LOSS_STATE) // a winning move!!!!
					{
						avail_threads[i].thread.join(); // should join immediately
						exit_reason = LOSS_STATE;
						search_continue = false;
						break;
					}
				}
			}
			if (threads_left == 0) // only way to reach here is if all the threads returned WIN_STATE
			{
				return_val = LOSS_STATE;
				break;
			}
		}
	}
	
	if (exit_reason == ERROR_STATE)
	{
		return_val = ERROR_STATE;
	}
	else if (exit_reason == LOSS_STATE)
	{
		return_val = WIN_STATE;
	}

	// clean up code
	for (uint_fast16_t i = 0; i < num_threads; i++) // for all of the threads that may still be running
	{
		if (avail_threads[i].avail_for_use == false) // if the thread is currently running/ hasn't been cleaned up yet
		{
			*(avail_threads[i].kill_flag) = true; // send the kill signal out
			avail_threads[i].thread.join(); // wait for it to join
		}
		thread_game_info_free(&avail_threads[i]); // and clean up all of its memory
	}

	return return_val;
}