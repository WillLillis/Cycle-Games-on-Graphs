#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <filesystem> // Property Pages->Configuration Properties->General->C++ Language Standard->ISO C++ 17 Standard (/std:c++17) (OR NEWER)
#include "Adjacency_Matrix.h"
#include "Misc.h"

/*
* 
* Most of the code for the program's menus will be in this header, as
* the name suggests. This will mostly consist of just printing out choices
* and taking in user's choices for said choices
* 
* I'm pretty inexperienced (read, have no experience) in creating menus/ user interface
* type stuff, so a lot of this will probably be fairly ham-fisted
* 
*/

/*
* 
* Instead of completely hardcoding the graph choices for generation into our generate menu function,
* we'll only semi-hardcode the graph choices, hopefully making it easier for future contributors to 
* add graph types as the research and code base progresses
* 
*/

/*
* 
* We'll utilize the struct I've defined below to control what graphs are generate-able
* and with that how it interfaces with the rest of the code. 
* 
* Each instance of the struct corresponds to a graph family. The struct will hold a name
* for said family, as well as a pointer to a function that will generate an adjacency
* listing for it and write said listing to a file
* 
* In order to add a graph to generate in the code base, basically all you'll have to do
* besides writing the generating function will be to add an instance of the below struct
* representing it into the avail_graphs array, add a #define for the entry number in the
* array, and increase the NUM_GRAPH_FAMS definition accordingly
* 
* (TO BE IMPLEMENTED) Generated graphs will be placed in folders according to their graph
* family name
* 
*/
// have to declare (but not define) the functions up here so that they
// can be in the avail_graphs array, defined below
void user_generalized_petersen_gen();
void user_stacked_prism_gen();
void user_z_mn_gen();

typedef struct GRAPH_GEN_INFO {
	std::string graph_name;
	void (*gen_func)();
	//void (*gen_func)(FILE*, uint_fast16_t, uint_fast16_t); // old way, where generating functions were called directly
}GRAPH_GEN_INFO;

GRAPH_GEN_INFO avail_graphs[] = {
	GRAPH_GEN_INFO{"Generalized Petersen", user_generalized_petersen_gen},
	GRAPH_GEN_INFO{"Stacked Prism", user_stacked_prism_gen},
	GRAPH_GEN_INFO{"Z_m^n", user_z_mn_gen}
};

// If the order in the avail_graphs array is changed or a new entry is added, these
// #define's should be updated accordingly
#define NUM_GRAPH_FAMS		3
#define GEN_PET_ENTRY		0
#define STACKED_PRISM_ENTRY	1
#define Z_MN_ENTRY			2

// Leave commented out if you want files in the project's working directory
//#define ALT_ADJ_PATH			"C:\\Users\\willl\\Desktop\\Adjacency Information"	// (for example on my machine...)
//#define ALT_RESULT_PATH		"C:\\Users\\willl\\Desktop\\Results"				// ^

void print_game_results(GAME_STATE p1_result)
{
	printf("%s Wins!\n", p1_result == WIN_STATE ? "P1" : "P2");
}

// returns true if it finds the valid adjacency info directory, 
// false otherwise, 
// how do we want to indicate which graph family for the sub dir?
bool verify_adj_info_path(std::filesystem::path* adj_path, bool fail_on_create, uint_fast16_t sub_dir_graph_fam = NUM_GRAPH_FAMS)
{
	std::filesystem::path adj_path_temp;
	
#ifdef ALT_ADJ_PATH // if the user supplied their own directory for the adjacency files, use it
	adj_path = std::filesystem::path(ADJ_DIR);
#else // otherwise we'll do things in the project's current directory
	adj_path_temp = std::filesystem::current_path();
	adj_path_temp.append("Adjacency Information");
	// just add the graph fam name here with an append?-> won't know which failed, but maybe that's ok
#endif // ALT_ADJ_PATH

	std::filesystem::directory_entry adj_dir(adj_path_temp);

	if (!adj_dir.exists()) // if we can't find the directory....
	{
#ifdef ALT_ADJ_PATH // if the user supplied their own directory for the adjacency files, tell them there's something wrong with it
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Unable to find the \"Adjacency Information\" directory.\nThe alternate directory variable is defined, make sure you supplied a valid path.\nRequested path: %s", ALT_ADJ_PATH);
		return false;
#else
		display_error(__FILE__, __LINE__, __FUNCSIG__, false,
			"Unable to find the \"Adjacency Information\" directory.\nThe alternate directory variable is not defined, the search was completed in the project's current directory.");
		printf("Creating the necessary directory now...\n");
		if (!std::filesystem::create_directory(adj_path_temp))
		{
			display_error(__FILE__, __LINE__, __FUNCSIG__, true,
				"Failed to create the specified \"Adjacency Information\" directory\nRequested path: %s", adj_path_temp.string().c_str());
			return false;
		}
#endif // ALT_ADJ_PATH
		printf("Press [ENTER] to continue...\n");
		char throw_away = std::getchar();
		return !fail_on_create; // if fail_on_create is true, we need to return false
	}

	// otherwise the Adjacency Information directory exists...now it's time to check for graph family-specific sub directories
	std::filesystem::path graph_sub_path;
	if (sub_dir_graph_fam >= 0 && sub_dir_graph_fam < NUM_GRAPH_FAMS) // if the specified graph family parameter is valid
	{
		graph_sub_path = adj_path_temp;
		graph_sub_path.append(avail_graphs[sub_dir_graph_fam].graph_name);
		std::filesystem::directory_entry graph_sub_dir(graph_sub_path);
		if (graph_sub_dir.exists())
		{
			*adj_path = graph_sub_path;
			return true;
		}
	}
	
	// if it's invalid or if the graph family's folder doesn't exist, just stick the file in the adjacency 
	*adj_path = adj_path_temp;
	return true;
}

// returns true if it finds the valid results directory, and assigns it as the value to the param passed in
// false otherwise, 
bool verify_results_path(std::filesystem::path* result_path, bool fail_on_create)
{
	std::filesystem::path result_path_temp;

#ifdef ALT_RESULT_PATH
	result_path = std::filesystem::path(ALT_RESULT_PATH);
#else
	result_path_temp = std::filesystem::current_path();
	result_path_temp.append("Results");
#endif // ALT_RESULT_PATH

	std::filesystem::directory_entry result_dir(result_path_temp);

	if (!result_dir.exists()) // if we can't find the directory...
	{
#ifdef ALT_RESULT_PATH // if the user supplied their own directory for their results, tell them there's something wrong with it
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Unable to find the \"Results\" directory.\nThe alternate directory variable is defined, make sure you supplied a valid path.\n Requested path: %s", result_path.string().c_str());
		return false;
#else
		printf("WARNING: Unable to find the \"Results\" directory.\n");
		printf("The alternate directory variable is not defined, the search was completed in the project's current directory.\n"); // otherwise we'll do things in the project's current directory
		printf("Creating the necessary directory now...\n");
		if (!std::filesystem::create_directory(result_path_temp)) // if the call failed to create the directory....
		{
			display_error(__FILE__, __LINE__, __FUNCSIG__, false,
				"Failed to create the specified \"Results\" directory\nRequested path: %s", result_path_temp.string().c_str());
			return false;
		}
		else // otherwise we're good 
		{
			printf("Done.\nPress [ENTER] to continue\n");
			char throw_away = std::getchar();
			*result_path = result_path_temp;
			return !fail_on_create;
		}
#endif // ALT_RESULT_PATH
	}
	// otherwise, the directory exists and we're fine to proceed
	*result_path = result_path_temp;
	return true;
}

// this is kind of long...look for ways to break up?
// want to change [BACK] options to go back a step in param selection, instead of back to the file selection page?
// also needs LOTS of testing....
void user_plays(std::filesystem::path adj_info_path)
{
	if (!std::filesystem::directory_entry(adj_info_path).exists())
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Supplied adjacency info file was not found/does not exist\nRequested path: %s", adj_info_path.string().c_str());
		return;
	}

	uint_fast16_t num_nodes = 0; 
	uint_fast16_t* adj_info = load_adjacency_info(adj_info_path, &num_nodes);
	if (adj_info == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Recieved invalid memory address after attempting to load adjacency information");
		return;
	}
	if (!(num_nodes > 0))
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Recieved invalid graph parameter (number of graphs nodes) after attempting to load adjacency information");
		return;
	}

	// prompt user for game (MAC or AAC)
	bool bad_input = false;
	std::string game_select_raw;
	uint_fast16_t game_select = 3;
	system("cls");
	printf("Select the game to play:\n");
	printf("[0] MAC\n");
	printf("[1] AAC\n");
	printf("[2] [BACK]\n");
	do
	{
		if (bad_input)
		{
			erase_lines(2);
		}
		bad_input = true;
		std::cin >> game_select_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(game_select_raw))
		{
			continue;
		}
		game_select = std::stoul(game_select_raw, NULL);
		if (game_select == 2) // [BACK] option
		{
			return;
		}
	} while (!(game_select >= 0 && game_select <=2));

	// prompt user for quiet vs. loud
	bad_input = false;
	std::string output_select_raw;
	uint_fast16_t output_select = 3;
	printf("Quiet or Loud:\n");
	printf("[0] Quiet\n");
	printf("[1] Loud\n");
	printf("[2] [BACK]\n");
	do
	{
		if (bad_input)
		{
			erase_lines(2);
		}
		bad_input = true;
		std::cin >> output_select_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(output_select_raw))
		{
			continue;
		}
		output_select = std::stoul(output_select_raw, NULL);
		if (output_select == 2) // [BACK] option
		{
			return;
		}
	} while (!(output_select >= 0 && output_select <= 2));

	// if the user asked for a loud run, make sure the results directory is all set up
	
	
	std::filesystem::path result_path;
	if (output_select == 1)
	{
		//std::filesystem::directory_entry dir;
		if (!verify_results_path(&result_path, false))
		{
			display_error(__FILE__, __LINE__, __FUNCSIG__, true,
				"Failed to find and/ or create the \"Results\" sub directory.");
			return;
		}
	}

	// prompt user for starting node on graph
	bad_input = false;
	std::string node_select_raw;
	uint_fast16_t node_select = num_nodes;
	printf("Select the starting node:\n");
	printf("[0 - %hhu] Said node\n", num_nodes - 1);
	printf("[%hhu] [BACK]\n", num_nodes);
	do
	{
		if (bad_input)
		{
			erase_lines(2);
		}
		bad_input = true;
		std::cin >> node_select_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(node_select_raw))
		{
			continue;
		}
		node_select = std::stoul(node_select_raw, NULL);
		if (node_select == num_nodes) // [BACK] option
		{
			return;
		}
	} while (!(output_select >= 0 && output_select < num_nodes));

	
	uint_fast16_t* edge_use = (uint_fast16_t*)calloc(num_nodes * num_nodes, sizeof(uint_fast16_t));
	uint_fast16_t* node_use = (uint_fast16_t*)calloc(num_nodes, sizeof(uint_fast16_t));
	if (edge_use == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Memory allocation error! Requested %zu bytes for the \"edge_use\" variable", num_nodes * num_nodes * sizeof(uint_fast16_t));
		if (node_use != NULL)
		{
			free(node_use);
		}
		return;
	}
	if (node_use == NULL)
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Memory allocation error! Requested %zu bytes for the \"node_use\" variable", num_nodes * sizeof(uint_fast16_t));
		if (edge_use != NULL)
		{
			free(edge_use);
		}
		return;
	}

	GAME_STATE game_result;
	if (output_select == 0) // Quiet 
	{
		if (game_select == 0) // MAC
		{
			game_result = play_MAC_quiet(node_select, num_nodes, adj_info, edge_use, node_use);
		}
		else // AAC
		{
			game_result = play_AAC_quiet(node_select, num_nodes, adj_info, edge_use, node_use);
		}
	}
	else // Loud
	{
		uint_fast16_t* move_hist = (uint_fast16_t*)malloc(num_nodes * sizeof(uint_fast16_t));
		if (move_hist == NULL)
		{
			display_error(__FILE__, __LINE__, __FUNCSIG__, true,
				"Memory allocation error! Requested %zu bytes for the \"move_hist\" variable", num_nodes * sizeof(uint_fast16_t));
			if (node_use != NULL)
			{
				free(node_use);
			}
			if (edge_use != NULL)
			{
				free(edge_use);
			}
			return;
		}

		FILE* result_stream;
		std::string file_name = adj_info_path.stem().string();
		file_name.append(game_select == 0 ? " -MAC- " : " -AAC- ");
		file_name.append("SN "); // SN for "Starting Node"
		file_name.append(std::to_string(node_select));
		file_name.append(".txt");
		result_path.append(file_name);
		// want to add some sort of versioning here so we don't automatically overwrite old files?

		// not sure if _set_errno() can be called on a non-Windows machine
//#ifdef WIN32 // non-Windows way to set errno?
		_set_errno(0); // "Always clear errno by calling _set_errno(0) immediately before a call that may set it"
//#endif // WIN32
		errno_t err = fopen_s(&result_stream, result_path.string().c_str(), "w");
		if (err != 0)
		{
#ifdef WIN32
			char err_buff[94]; // Your string message can be, at most, 94 characters long. (https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-s-strerror-s-wcserror-s-wcserror-s?view=msvc-170)
			strerror_s(err_buff, 94, NULL);
			display_error(__FILE__, __LINE__, __FUNCSIG__, true,
				"Failed to open the result output file.\nRequested path: %s\nfopen_s error message: %s", result_path.string().c_str(), err_buff);
#else
			display_error(__FILE__, __LINE__, __FUNCSIG__, true,
				"Failed to open the result output file.\nRequested path: %s\nfopen_s error code: %d", result_path.string().c_str(), err);
#endif // WIN32

			if (node_use != NULL)
			{
				free(node_use);
			}
			if (edge_use != NULL)
			{
				free(edge_use);
			}
			if (move_hist != NULL)
			{
				free(move_hist);
			}
			return;
		}
		if (game_select == 0) // MAC
		{
			game_result = play_MAC_loud(node_select, num_nodes, adj_info, edge_use, node_use, move_hist, 0, result_stream);
		}
		else // AAC
		{
			game_result = play_AAC_loud(node_select, num_nodes, adj_info, edge_use, node_use, move_hist, 0, result_stream);
		}

		if (move_hist != NULL)
		{
			free(move_hist);
		}
		if (result_stream != NULL) // if result_stream is NULL, then we don't need to close it?
		{
			int close_err = fclose(result_stream);
			if (close_err != 0)
			{
				display_error(__FILE__, __LINE__, __FUNCSIG__, false,
					"Failed to properly close the output file.\nPath associated with file stream: %s", result_path.string().c_str());
			}
		}
	}

	if (adj_info != NULL)
	{
		free(adj_info);
	}
	if (edge_use != NULL)
	{
		free(edge_use);
	}
	if (node_use != NULL)
	{
		free(node_use);
	}
	
	printf("\n\nFile: %s, Starting Node: %hhu, Game: %s\n", adj_info_path.filename().string().c_str(), node_select, game_select == 0 ? "MAC" : "AAC");
	print_game_results(game_result);

	printf("Press [ENTER] to continue\n");
	char throw_away = std::getchar();
}

// do we want a file naming option?
void play_menu_subdir(std::filesystem::path curr_dir)
{
	if (!std::filesystem::directory_entry(curr_dir).exists())
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Failed to find the specified subdirectory.\nRequested path: %s", curr_dir.string().c_str());
		return;
	}

	uint_fast16_t entry_index = 0;
	std::filesystem::path temp_path = curr_dir;
	bool bad_selection = false;
	uint_fast16_t file_selection = entry_index;
	std::string file_selection_raw;

	while (true)
	{
		system("cls");
		// is having the full path helpful?
		printf("%s\n", curr_dir.string().c_str()); // print the current directory so the user knows where they are
		printf("Select a [FILE] to play on, or a [DIR] to open.\n");
		entry_index = 0;

		for (auto const& dir_entry : std::filesystem::directory_iterator(curr_dir)) 
		{
			if (dir_entry.is_directory())
			{
				printf("[%03hhu][DIR]  %s\n", entry_index, dir_entry.path().filename().string().c_str());
				entry_index++;
			}
			else if (dir_entry.is_regular_file())
			{
				printf("[%03hhu][FILE] %s\n", entry_index, dir_entry.path().filename().string().c_str());
				entry_index++;
			}
		}
		if (entry_index == 0)
		{
			printf("<No valid entries>\n");
		}
		printf("[%03hhu][BACK]\n", entry_index);

		file_selection = entry_index + 1; // initially unacceptable value 
		file_selection_raw;
		bad_selection = false;
		// way to clear only the last line?
		do
		{
			if (bad_selection)
			{
				erase_lines(2); // need to erase 2 lines because hitting enter puts us on a newline, and then we have to erase the original bad line
			}
			bad_selection = true;
			std::cin >> file_selection_raw;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			if (!is_number(file_selection_raw))
			{
				continue;
			}
			file_selection = std::stoul(file_selection_raw, NULL);
			if (file_selection == entry_index) // [BACK] option
			{
				return;
			}
		} while (!(file_selection >= 0 && file_selection < entry_index));

		// once we're past that loop, the file_selection value represents a file or directory to open...
		// reading through the C++ documentation, I didn't find a way to directly access a member of the directory
		// using the std::filesystem functions..., guess we'll just iterate through until we get to the right one
		entry_index = 0;
		temp_path = curr_dir;
		for (auto const& dir_entry : std::filesystem::directory_iterator(curr_dir)) // starts in cwd
		{
			if (dir_entry.is_directory())
			{
				if (entry_index == file_selection)
				{
					temp_path /= dir_entry.path().filename();
					play_menu_subdir(temp_path);
					break;
				}
				else
				{
					entry_index++;
				}
			}
			else if (dir_entry.is_regular_file())
			{
				if (entry_index == file_selection)
				{
					user_plays(dir_entry.path()); // necessary user_plays function call->TO BE IMPLEMENTED
					break;
				}
				else
				{
					entry_index++;
				}
			}
		}
	}
}
// change directory verification to a function call?
	// function will either search for user defined path or path in curr dir
void play_menu()
{
	system("cls");
	std::filesystem::path adj_path;
#ifdef ALT_ADJ_PATH // if the user supplied their own directory for the adjacency files, use it
	adj_path = ADJ_DIR;
#else // otherwise we'll do things in the project's current directory
	adj_path = std::filesystem::current_path(); 
	adj_path.append("Adjacency Information");
#endif // ALT_ADJ_PATH

	if (!verify_adj_info_path(&adj_path, true))
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Failed to find a non-empty \"Adjacency Information\" directory.\nRequested path: %s", adj_path.string().c_str());
	}

	std::filesystem::directory_entry adj_dir(adj_path);

	play_menu_subdir(adj_dir);
}

// Need to be able to walk through a variable number of graph parameters...think it's safe to assume they're all uint_fast16_t's, can make a change later if it's needed
std::string get_adj_info_file_name(uint_fast16_t graph_fam, uint_fast8_t num_args, ...)
{
	if (!(graph_fam >= 0 && graph_fam < NUM_GRAPH_FAMS))
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true, 
			"Invalid graph family parameter supplied to generate the file name.");
	}

	std::string file_name = avail_graphs[graph_fam].graph_name;
	if (num_args == 0) // no graph parameters
	{
		file_name.append(".txt");
		return file_name;
	}

	file_name.append(" (");

	va_list arg_ptr;
	va_start(arg_ptr, num_args);
	for (uint_fast16_t curr_arg = 0; curr_arg < num_args; curr_arg++)
	{
		if (curr_arg > 0)
		{
			file_name.append(", ");
		}
		file_name.append(std::to_string(va_arg(arg_ptr, uint_fast16_t)));
	}
	va_end(arg_ptr);
	file_name.append(").txt");
	
	return file_name;
}

// We'll define the some generating functions to be called by the user that will act as wrappers for the true generating functions
// These functions will attempt to open the file to write to, take in any necessary parameters from the user, 
// call the actual generating function, and then close the file after it has finished its work 
// these will also perform any necessary parameter checks 
// Organizationally these functions are in this file (instead of Adjacency_Matrix.h) because they're more of a direct result in
// how I'm implementing the menu than how the graphs are generated

// TODO:
// Need to go through file name creation bits of code so that these save in the correct folders in whatever adjacency directory is specified
// If a subfolder of the correct name doesn't exist, just stick it in the directory-> leave it up to the user where they want their files
// If a subfolder of the correct name does exist, put it in there->organization done for those who don't care
	// do we want to add option to play immediately on a file we jsut generated?->talk with Gates/ Kelvey

void user_generalized_petersen_gen()
{
	// Generalized Petersen graphs need 'n' and 'k' parameters to be contructed
	uint_fast16_t n_param = 0; // providing initial assignment to get uninitialized local variable warning to go away
	uint_fast16_t k_param = 0; // ^
	std::string n_raw, k_raw;

	do 
	{
		system("cls");
		printf("Provide the following parameters for the construction of the %s graph.\n",
			avail_graphs[GEN_PET_ENTRY].graph_name.c_str());

		printf("(The number of nodes in each of the graph's two \"rings\")\n");
		printf("n: ");
		std::cin >> n_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(n_raw))
		{
			continue;
		}
		n_param = std::stoul(n_raw, NULL);

		printf("(Distance between connections for the inner ring's nodes)\n");
		printf("k: ");
		std::cin >> k_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(k_raw))
		{
			continue;
		}
		k_param = std::stoul(k_raw, NULL);
	} while (!(n_param >= 3) 
		|| !(k_param >= 1) 
		|| !(k_param <= ((n_param - 1) / 2)));
	
	printf("Generating graph...");
	FILE* output;

	std::filesystem::path output_path;
	if (!verify_adj_info_path(&output_path, false, GEN_PET_ENTRY))
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Issue found when checking the adjacency information path.");
		return;
	}

	std::string file_name = get_adj_info_file_name(GEN_PET_ENTRY, 2, n_param, k_param);
	output_path.append(file_name);

	// can check if file exists here if we want to do some kind of versioning....

	// not sure if _set_errno() can be called on a non-Windows machine
//#ifdef WIN32 // non-Windows way to set errno?
	_set_errno(0); // "Always clear errno by calling _set_errno(0) immediately before a call that may set it"
//#endif // WIN32
	errno_t err = fopen_s(&output, output_path.string().c_str(), "w");
	if (err != 0) // Will need to tweak error reporting once we get the generated graphs in the correct directory
	{
#ifdef WIN32
		char err_buff[94]; // Your string message can be, at most, 94 characters long. (https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-s-strerror-s-wcserror-s-wcserror-s?view=msvc-170)
		strerror_s(err_buff, 94, NULL);
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Failed to open the result output file.\nRequested path: %s\nfopen_s error message: %s", output_path.string().c_str(), err_buff);
#else
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Failed to open the result output file.\nRequested path: %s\nfopen_s error code: %d", output_path.string().c_str(), err);
#endif // WIN32
		return;
	}

	generalized_petersen_gen(output, n_param, k_param); // call the actual generation function
	if (output != NULL)
	{
		int close_err = fclose(output);
		if (close_err != 0)
		{
			display_error(__FILE__, __LINE__, __FUNCSIG__, true,
				"Failed to properly close the output file.\nPath associated with file stream: %s", output_path.string().c_str());
			return;
		}
	}

	printf("Generation completed.\n"); 
	printf("Press [ENTER] to continue\n");
	char throw_away = std::getchar();
}

void user_stacked_prism_gen()
{
	// Stacked Prism graphs need 'm' and 'n' parameters to be contructed
	uint_fast16_t m_param = 0; // providing initial assignment to get uninitialized local variable warning to go away
	uint_fast16_t n_param = 0; // ^
	std::string m_raw, n_raw;

	do
	{
		system("cls");
		printf("Provide the following parameters for the construction of the %s graph.\n",
			avail_graphs[GEN_PET_ENTRY].graph_name.c_str());

		printf("(The number of nodes in each concentric ring)\n");
		printf("m: ");
		std::cin >> m_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(m_raw))
		{
			continue;
		}
		m_param = std::stoul(m_raw, NULL);

		printf("(The number of concentric rings)\n");
		printf("n: ");
		std::cin >> n_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(n_raw))
		{
			continue;
		}
		n_param = std::stoul(n_raw, NULL);
	} while (!(m_param >= 3)
		|| !(n_param >= 1));

	printf("Generating graph...");
	FILE* output;

	std::filesystem::path output_path;
	if (!verify_adj_info_path(&output_path, false, STACKED_PRISM_ENTRY))
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Issue found when checking the adjacency information path.");
		return;
	}

	std::string file_name = get_adj_info_file_name(STACKED_PRISM_ENTRY, 2, m_param, n_param);
	output_path.append(file_name);

	// not sure if _set_errno() can be called on a non-Windows machine
//#ifdef WIN32 // non-Windows way to set errno?
	_set_errno(0); // "Always clear errno by calling _set_errno(0) immediately before a call that may set it"
//#endif // WIN32
	errno_t err = fopen_s(&output, output_path.string().c_str(), "w");
	if (err != 0) // Will need to tweak error reporting once we get the generated graphs in the correct directory
	{
#ifdef WIN32
		char err_buff[94]; // Your string message can be, at most, 94 characters long. (https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-s-strerror-s-wcserror-s-wcserror-s?view=msvc-170)
		strerror_s(err_buff, 94, NULL);
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Failed to open the result output file.\nRequested path: %s\nfopen_s error message: %s", file_name.c_str(), err_buff);
#else
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Failed to open the result output file.\nRequested path: %s\nfopen_s error code: %d", file_name.c_str(), err);
#endif // WIN32
		return;
	}

	stacked_prism_gen(output, m_param, n_param); // call the actual generation function

	if (output != NULL)
	{
		int close_err = fclose(output);
		if (close_err != 0)
		{
			display_error(__FILE__, __LINE__, __FUNCSIG__, true,
				"Failed to properly close the output file.\nPath associated with file stream: %s", file_name.c_str());
			return;
		}
	}

	printf("Generation completed.\n");
	printf("Press [ENTER] to continue\n");
	char throw_away = std::getchar();
}

void user_z_mn_gen()
{
	// Z_m^n graphs need 'm' and 'n' parameters to be contructed
	uint_fast16_t m_param = 0; // providing initial assignment to get uninitialized local variable warning to go away
	uint_fast16_t n_param = 0; // ^
	std::string m_raw, n_raw;

	do
	{
		system("cls");
		printf("Provide the following parameters for the construction of the %s graph.\n",
			avail_graphs[Z_MN_ENTRY].graph_name.c_str());

		printf("(Tuple entries ranging from 0 to m-1)\n");
		printf("m: ");
		std::cin >> m_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(m_raw))
		{
			continue;
		}
		m_param = std::stoul(m_raw, NULL);

		printf("(n entries in the tuple)\n");
		printf("n: ");
		std::cin >> n_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(n_raw))
		{
			continue;
		}
		n_param = std::stoul(n_raw, NULL);
	} while (!(m_param >= 3) // are they really this constrained?
		|| !(n_param >= 1));

	printf("Generating graph...");
	FILE* output;

	std::filesystem::path output_path;
	if (!verify_adj_info_path(&output_path, false, Z_MN_ENTRY)) 
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Issue found when checking the adjacency information path.");
		return;
	}

	std::string file_name = get_adj_info_file_name(Z_MN_ENTRY, 2, m_param, n_param);
	output_path.append(file_name);
	
	// not sure if _set_errno() can be called on a non-Windows machine
//#ifdef WIN32 // non-Windows way to set errno?
	_set_errno(0); // "Always clear errno by calling _set_errno(0) immediately before a call that may set it"
//#endif // WIN32
	errno_t err = fopen_s(&output, output_path.string().c_str(), "w");
	if (err != 0) // Will need to tweak error reporting once we get the generated graphs in the correct directory
	{
#ifdef WIN32
		char err_buff[94]; // Your string message can be, at most, 94 characters long. (https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-s-strerror-s-wcserror-s-wcserror-s?view=msvc-170)
		strerror_s(err_buff, 94, NULL);
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Failed to open the result output file.\nRequested path: %s\nfopen_s error message: %s", file_name.c_str(), err_buff);
#else
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"Failed to open the result output file.\nRequested path: %s\nfopen_s error code: %d", file_name.c_str(), err);
#endif // WIN32
		return;
	}

	z_mn_gen(output, m_param, n_param); // call the actual generation function
	
	if (output != NULL)
	{
		int close_err = fclose(output);
		if (close_err != 0)
		{
			display_error(__FILE__, __LINE__, __FUNCSIG__, true,
				"Failed to properly close the output file.\nPath associated with file stream: %s", file_name.c_str());
			return;
		}
	}

	printf("Generation completed.\n");
	printf("Press [ENTER] to continue\n");
	char throw_away = std::getchar();
}

void generate_menu()
{
	std::string graph_choice_raw; // could just use a char since we have less than 10 choices, but this allows for easier expansion in the future
	uint_fast16_t graph_choice = NUM_GRAPH_FAMS + 1; // initialize to unacceptable value for weird edge case
	
	while (true)
	{
		graph_choice = NUM_GRAPH_FAMS + 1;
		do 
		{
			system("cls");
			printf("Select which type of graph you'd like to generate an adjacency listing for.\n");
			for (uint_fast16_t curr_choice = 0; curr_choice < NUM_GRAPH_FAMS; curr_choice++)
			{
				printf("[%hhu] %s\n", curr_choice, avail_graphs[curr_choice].graph_name.c_str());
			}
			printf("[%u] BACK\n", NUM_GRAPH_FAMS);

			std::cin >> graph_choice_raw;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			if (!is_number(graph_choice_raw))
			{
				continue;
			}
			graph_choice = std::stoul(graph_choice_raw, NULL);
			if (graph_choice == NUM_GRAPH_FAMS) // BACK option
			{
				return;
			}
		} while (!(graph_choice >= 0 && graph_choice < NUM_GRAPH_FAMS));

		// call appropriate generating function
		avail_graphs[graph_choice].gen_func();
	}
}

void main_menu()
{
	// what do we want to do with the welcome screen?
	printf("Welcome to the Cycle Games on Graphs Research Tool!\n");
	printf("This code was written and developed by <How do we wanna do names here?>\n");
	printf("Press [ENTER] to continue\n");

	char throw_away = std::getchar(); 

	std::string raw_menu_choice;
	uint_fast16_t menu_choice = 3; // initialize to unacceptable value for weird edge case
	while (true)
	{
		menu_choice = 3; // have to reset this value for new pass
		do
		{
			system("cls");
			
			printf("[0] Play a game\n");
			printf("[1] Generate an Adjacency Listing\n");
			printf("[2] EXIT\n");

			std::cin >> raw_menu_choice;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			if (!is_number(raw_menu_choice))
			{
				continue; 
			}
			menu_choice = std::stoul(raw_menu_choice, NULL);
			if (menu_choice == 2)
			{
				return;
			}

		} while (!(menu_choice >= 0 && menu_choice <= 2));

		if (menu_choice == 0)
		{
			play_menu();
		}
		else if (menu_choice == 1)
		{
			generate_menu();
		}
	}
}
