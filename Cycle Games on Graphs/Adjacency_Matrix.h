#pragma once
/*
* 
* This file will mostly hold the code that deals with generating adjacency listings/ matrices
* and writing them to files, as well as the reading of said files for use by the game playing code
* 
*/

#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <cassert>
#include <filesystem>
#include <cmath>
#include "Misc.h"

// Maybe a little overkill, but some defensive programming here to limit how many elements we'll read in
#define MAX_ELEMENTS	500 // arbitrary max value, feel free to increase if needed

// Used within Cycle_Games.h, but made sense categorically to #define these in here
typedef uint_fast8_t Adjacency_Info;
#define ADJACENT		1
#define NOT_ADJACENT	0

/****************************************************************************
* load_adjacency_info
*
* - Opens the specified file containing an adjacency listing, then allocates
* the space for and fills a buffer for the adjacency matrix corresponding to 
* the listing in the file
* - While it would be more straightforward to repeatedly call std::getline as
* opposed to reading in the entire block all at once, having one large read 
* from the disk should be faster than a bunch of smaller reads
*
* Parameters :
* - file_path : string holding the path to the desired file
* - num_nodes_out : passed in by reference in order to tell the caller how
* many nodes the given graph has
*
* Returns :
* - uint_fast16_t* : pointer to a buffer of size 
* (*num_nodes_out) * (*num_nodes_out) holding the adjacency matrix
****************************************************************************/
// Might want to add functionality for reading in adjacency matrices directly later on
// Need to add ability to read in adjacency matrices
uint_fast16_t* load_adjacency_info(std::filesystem::path file_path, uint_fast16_t* num_nodes_out)
{
	std::fstream data_stream;

	data_stream.open(file_path.string().c_str(), std::fstream::in); // open file with read permissions only
	if (!data_stream.is_open())
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Failed to open the adjacency information file.\nRequested path: %s", file_path.string().c_str());
	}

	size_t file_length = get_file_length(&data_stream);

	// allocate a buffer with the length to read all of the file's contents in at once
	if (!(file_length > 0))
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Received an invalid file length.\nRequested path: %s", file_path.string().c_str());
		return NULL;
	}
	char* data = (char*)malloc(file_length);
	if (data == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Memory allocation error! Requested %zu bytes for the \"file_length\" variable", file_length);
		return NULL;
	}

	data_stream.read(data, file_length); // read the file's contents into the data buffer
	data_stream.close();

	uint_fast16_t temp_label;
	uint_fast16_t max_label = 0;
	// Need to find the largest node number in the data so that we can allocate the buffer for the adjacency matrix
	// skip over the labels in the first line
	size_t data_start = 0;
	while (data_start < file_length && data[data_start] != '\n')
	{
		data_start++;
	}
	data_start++; // increment one more time to get past that first newline character
	size_t curr = data_start;

	assert(curr < file_length); // assert necessary to make VS warning go away
	while (curr < file_length)
	{
		if (std::isdigit(data[curr]))
		{
			temp_label = std::atoi(data + curr);
			max_label = std::max(temp_label, max_label);
			curr += num_digits(temp_label) + 1; // advance to the next character, and then one more to skip a comma/ newline character
			continue; // back to the top of the loop
		}
		else // something went wrong, return a NULL pointer to indicate an error
		{
			display_error(__FILE__, __LINE__, __FUNCSIG__, true,
				"Error parsing the file searching for the largest node label.");
			free(data);
			return NULL;
		}
	}

	max_label++; // have to account for 0 based counting with the node labels
	*num_nodes_out = max_label;
	// now that we have the max label, we can allocate an array for the adjacency matrix
	uint_fast16_t* adj_matrix = (uint_fast16_t*)calloc(sizeof(uint_fast16_t), (size_t)max_label * (size_t)max_label);
	if (adj_matrix == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Memory allocation error! Requested %zu bytes for the \"adj_matrix\" variable", 
			sizeof(uint_fast16_t) * (size_t)max_label * (size_t)max_label);
		if (data != NULL)
		{
			free(data);
		}
		return NULL;
	}

	uint_fast16_t node_1, node_2; // to temporarily store node values read in from the data block
	bool second_node = false; // to track which node (first or second on a given line) we're on

	curr = data_start;
	if (!(curr < file_length))
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Issue parsing the adjacency information file loaded into memory.\n File path: %s",
			file_path.string().c_str());
		if (data != NULL)
		{
			free(data);
		}
		if (adj_matrix != NULL)
		{
			free(adj_matrix);
		}
		return NULL;
	}
	while (curr < file_length)
	{
		if (std::isdigit(data[curr]))
		{
			if (!second_node) // reading in the first digit in a pair
			{
				node_1 = std::atoi(data + curr);
				curr += num_digits(node_1); // advance to the first char past that of the number's
				// we shouldn't be at the end of the file, AND the next character has to be a comma
				if (!(curr < file_length && data[curr] == ','))
				{
					display_error(__FILE__, __LINE__, __FUNCSIG__, true,
						"Issue parsing the adjacency information file loaded into memory.\n File path: %s",
						file_path.string().c_str());
					if (data != NULL)
					{
						free(data);
					}
					if (adj_matrix != NULL)
					{
						free(adj_matrix);
					}
					return NULL;
				}
				curr++; // advance one more character to skip the comma separating the two digits
				second_node = true;
				continue;
			}
			else // reading in the second digit
			{
				node_2 = std::atoi(data + curr);
				curr += num_digits(node_2); // advance to the first char past that of the number's
				if (curr < file_length) // if we're not at the end of the file...
				{
					if (!(data[curr] == '\n')) // ...there should be a newline character next
					{
						display_error(__FILE__, __LINE__, __FUNCSIG__, true,
							"Issue parsing the adjacency information file loaded into memory.\n File path: %s",
							file_path.string().c_str());
						if (data != NULL)
						{
							free(data);
						}
						if (adj_matrix != NULL)
						{
							free(adj_matrix);
						}
						return NULL;
					}
				}
				curr++; // advance one more character to skip the newline char
				adj_matrix[index_translation(max_label, node_1, node_2)] = ADJACENT;
				adj_matrix[index_translation(max_label, node_2, node_1)] = ADJACENT;
				second_node = false;
				continue;
			}
		}
		else // something went wrong, return a NULL pointer to indicate an error
		{
			display_error(__FILE__, __LINE__, __FUNCSIG__, true,
				"Issue parsing the adjacency information file loaded into memory.\n File path: %s",
				file_path.string().c_str());
			if (data != NULL)
			{
				free(data);
			}
			if (adj_matrix != NULL)
			{
				free(adj_matrix);
			}
			return NULL;
		}
	}

	if (data != NULL)
	{
		free(data);
	}
	return adj_matrix;
}

// Generating functions
// do we want to change these so that they dump all their info to a buffer,
// then write it all at once?-> won't matter as much as the reads because write buffering is a thing
// but may still be the best practice
// would need to develop means to calculating the required buffer size beforehand....
/****************************************************************************
* generalized_petersen_gen
*
* - Writes an adjacency listing for a generalized petersen graph of the 
* specified parameters to the file stream
* - https://mathworld.wolfram.com/GeneralizedPetersenGraph.html
*
* Parameters :
* - output : file stream to write the adjacency listing to
* - n : graph parameter
*	- number of nodes on a ring in the graph
* - k : graph parameter
*	- how many nodes away the inner rings are connected 
*
* Returns :
* - none
****************************************************************************/
void generalized_petersen_gen(FILE* output, uint_fast16_t n, uint_fast16_t k)
{
	if (output == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Invalid file stream.");
		return;
	}
	fprintf(output, "Adjacency_Listing\n"); // label at the top of the file so we know what it is

	// outer ring connections to adjacent nodes around the ring
	for (uint_fast16_t i = 0; i < n; i++)
	{
		fprintf(output, "%hhu,%hhu\n", i, (i + 1) % n);
	}
	// outer ring to inner ring "spokes"
	for (uint_fast16_t i = 0; i < n; i++)
	{
		fprintf(output, "%hhu,%hhu\n", i, i + n);
	}
	// inner ring connections (ew)
	for (uint_fast16_t i = n; i < (2 * n) - 1; i++)
	{
		fprintf(output, "%hhu,%hhu\n", i, ((i + k) % n) + n);
	}
	// also an inner ring connection
	fprintf(output, "%hhu,%hhu", (2 * n) - 1, ((((2 * n) - 1) + k) % n) + n); // last entry does NOT get a newline char after it

}

/****************************************************************************
* stacked_prism_gen
*
* - Writes an adjacency listing for a stacked prism graph of the
* specified parameters to the file stream
* - https://mathworld.wolfram.com/StackedPrismGraph.html
*
* Parameters :
* - output : file stream to write the adjacency listing to
* - n : graph parameter
*	- number of nodes on a ring in the graph
* - m : graph parameter
*	- how many stacked prisms the graph has
*
* Returns :
* - none
****************************************************************************/
void stacked_prism_gen(FILE* output, uint_fast16_t m, uint_fast16_t n)
{
	if (output == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Invalid file stream.");
		return;
	}
	fprintf(output, "Adjacency_Listing\n"); // label at the top of the file so we know what it is

	for (uint_fast16_t i = 0; i < n; i++)
	{
		for (uint_fast16_t j = 0; j < m - 1; j++)
		{
			fprintf(output, "%hhu,%hhu\n", j + (i * m), j + 1 + (i * m));
		}
		fprintf(output, "%hhu,%hhu", m - 1 + (i * m), i * m);
		if (i != (n - 1)) // doing this to avoid placing a newline char at the end of the file
		{
			fprintf(output, "\n");
		}
		if (i < n - 1)
		{
			for (uint_fast16_t k = 0; k < m; k++)
			{
				fprintf(output, "%hhu,%hhu\n", k + (i * m), k + ((i + 1) * m));
			}
		}
	}
}

/*
* 
* - A brief discussion before the Z_m^n generation code
* - This is a bit gross. I remember talking with Dr. Gates
* regarding a good way to generate all the members of a given Z_m^n
* group, and the recursive solution is what we came up with
* - An iterative solution would likely be much clearer and vastly preferred, but for 
* now this works and we're moving on
* - The original implementation of this used std::vectors and a crappy little
* tuple template class I wrote 
*	- While it may make sense to switch back to using the template class later, for
*	now I'll just go with the approach below
* 
* - I initially tried to allocate an array of pointers, so that each entry pointed 
* to another block of memory where each tuple would live, but visual studio wouldn't
* stop giving me warnings, so I decided to do it with a single dimensional array, where 
* the tuples are just laid out next to eachother, and it's more reliant on the function
* using this stored info to know what's what
* 
*/

/****************************************************************************
* tuple_to_index
*
* - Given an array holding some number of tuples as adjacent entries, 
* calculates the index in said array for the specified entry within the 
* specified tuple
*
* Parameters :
* - n : number of entries per tuple
* - tuple_num : which tuple we're accessing within the array of tuples
* - tuple_entry : which entry we're accessing within said tuple
*
* Returns :
* - uint_fast16_t : the index in the array for said entry within the specified 
* tuple
****************************************************************************/
inline uint_fast16_t tuple_to_index(uint_fast16_t n, uint_fast16_t tuple_num, uint_fast16_t tuple_entry)
{
	return (n * tuple_num) + tuple_entry;
}

/****************************************************************************
* tuple_diff
*
* - Calculates the "distance" between two tuples
*	- Simply calculates the sum of the differences for each of the tuples' 
*	entries
* - The two tuples MUST be of the same length
*
* Parameters :
* - tuple_1 : pointer to the first entry of the first tuple
* - tuple_2 : pointer to the first entry of the second tuple
* - num_entries : the number of entries in the two tuples 
*	- corresponds to n in Z_m^n
*
* Returns :
* - uint_fast16_t : the distance between tuple_1 and tuple_2
****************************************************************************/
// Different/ better way to indicate error on return?-> make it a int_fast32_t, return -1 or something?
uint_fast16_t tuple_diff(uint_fast16_t* tuple_1, uint_fast16_t* tuple_2, uint_fast16_t num_entries)
{
	if (tuple_1 == NULL || tuple_2 == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Invalid memory address(es) supplied. Distance calculations will be invalid");
		return 0;
	}

	uint_fast16_t diff = 0;

	for (uint_fast16_t entry = 0; entry < num_entries; entry++)
	{
		// cast to regular uint16_t necessary, otherwise VS complains about 
		// "more than one instance of overloaded function matches the argument list
		diff += std::abs((int16_t)((int16_t)tuple_1[entry] - (int16_t)tuple_2[entry]));
	}
	return diff;
}

/****************************************************************************
* z_mn_group_gen
*
* - Helper function for z_mn_gen
* - Generates all the entries of a given Z_m^n group and places them in the 
* passed in array
*
* Parameters :
* - member_list : array of size n * m^n that holds every member of a given 
* Z_m^n group
*	- passed in by reference, so once it's filled, the caller has access
* - value_holder : temporary array that holds the "current value" of the 
* tuple being generated at various points within the tuple
*	- passed by reference so its value persists throughout recursive calls,
*	contents mean nothing to the caller
* - place_in_tuple : the current point in the n-tuple being filled
* - place_in_member_list : holds the current entry the the member_list array
* for the current tuple to be entered
*	 - holds the tuple number (in terms of the first tuple being generated, 
* the second, third, and so on....), not an actual address/ offset in bytes
*	- passed by reference so its value persists throughout recursive calls,
*	contents mean nothing to the caller
* - m : graph parameter
*	- each tuple entry can range from 0 to m-1
* - n : graph parameter
*	- the number of entries in each tuple
*
* Returns :
* - none
****************************************************************************/
void z_mn_group_gen(uint_fast16_t* member_list, uint_fast16_t* value_holder,
	uint_fast16_t place_in_tuple, uint_fast16_t* place_in_member_list, uint_fast16_t m, uint_fast16_t n)
{
	if (member_list == NULL || value_holder == NULL || place_in_member_list == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Invalid memory address(es) supplied. Group generation results will be invalid.");
		return;
	}

	if (place_in_tuple < n - 1)
	{
		for (uint_fast16_t i = 0; i < m; i++)
		{
			value_holder[place_in_tuple] = i;
			z_mn_group_gen(member_list, value_holder, place_in_tuple + 1, place_in_member_list, m, n);
		}
	}
	else
	{
		for (uint_fast16_t i = 0; i < m; i++)
		{
			value_holder[place_in_tuple] = i; // replace place_in_tuple with n-1 here?

			for (uint_fast16_t j = 0; j < n; j++)
			{
				// set the jth entry of the place_in_member_list tuple with the value of value_holder[j]
				member_list[tuple_to_index(n, *place_in_member_list, j)] = value_holder[j]; 
			}
			*place_in_member_list += 1; // move to the next tuple in our member_list array
		}
	}
}

/****************************************************************************
* z_mn_gen
*
* - Calls z_mn_group_gen() to generate all members of a given Z_m^n group,
* and then writes said group's adjacenecy listing to the supplied file stream
*
* Parameters :
* - output : the file stream to write the adjacency listing to
*	- passed in by reference, so once it's filled, the caller has access
* - m : graph parameter
*	- each tuple entry can range from 0 to m-1
* - n : graph parameter
*	- the number of entries in each tuple
*
* Returns :
* - none
****************************************************************************/
void z_mn_gen(FILE* output, uint_fast16_t m, uint_fast16_t n)
{
	if (output == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Invalid file stream.");
		return;
	}

	uint_fast16_t num_tuples = (uint16_t)std::pow(m, n);
	uint_fast16_t* value_holder = (uint_fast16_t*)malloc(n * sizeof(uint_fast16_t));;
	uint_fast16_t* member_list = (uint_fast16_t*)malloc((size_t)(num_tuples * n) * sizeof(uint_fast16_t));
	uint_fast16_t* place_in_member_list = (uint_fast16_t*)calloc(1, sizeof(uint_fast16_t)); // would just make this a stack variable and pass its address in, but was getting heap corruption warnings
	bool has_entry = false;

	if (value_holder == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Memory allocation error! Requested %zu bytes for the \"value_holder\" variable", n * sizeof(uint_fast16_t));
		return;
	}
	if (member_list == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Memory allocation error! Requested %zu bytes for the \"member_list\" variable", (size_t)(num_tuples * n) * sizeof(uint_fast16_t));
		return;
	}
	if (place_in_member_list == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Memory allocation error! Requested %zu bytes for the \"place_in_member_list\" variable", sizeof(uint_fast16_t));
		return;
	}
	
	z_mn_group_gen(member_list, value_holder, 0, place_in_member_list, m, n);

	if (value_holder != NULL)
	{
		free(value_holder);
	}
	if (place_in_member_list != NULL)
	{
		free(place_in_member_list);
	}
	
	// output to file
	fprintf(output, "Adjacency_Listing\n");
	for (uint_fast16_t i = 0; i < num_tuples; i++)
	{
		for (uint_fast16_t j = i; j < num_tuples; j++)
		{
			/* 
			* 
			* Quick note here, it looks like adding a number to the member_list pointer adds in
			* multiples of uint16_t's, so that's why I'm not adding the size of a tuple in bytes 
			* directly to the pointer, but rather how many uint16_t entries it should move over
			* 
			* I spent some time printing out the addresses for some test cases and it does 
			* appear to be working correctly at least
			* 
			* The specific warning was "expression mixes element counts and byte quantities"
			* 
			*/
			if (tuple_diff(member_list + (i * n), member_list + (j * n), n) == 1) 
			{
				if (has_entry == true) // doing this to avoid a newline character at the end of the file
				{
					fprintf(output, "\n%hhu,%hhu", i, j);
				}
				else
				{
					fprintf(output, "%hhu,%hhu", i, j);

				}
				
				has_entry = true;
			}
		}
	}
	
	if (member_list != NULL)
	{
		free(member_list);
	}
}