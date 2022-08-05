#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <filesystem> // Property Pages->Configuration Properties->General->C++ Language Standard->ISO C++ 17 Standard (/std:c++17) (OR NEWER)
#include "Adjacency_Matrix.h"

/*
* 
* Most of the code for the program's menus will be in this header, as
* the name suggests. This will mostly consist of just printing out choices
* and taking in user's choices for said choices
* 
* I'm pretty inexperienced (read, have no experience) in creating menus/ user interface
* type stuff, so a lot of this will probably be fairly ham-fisted
* 
* For some of the titles I may utilize the Big Char project, something I worked on earlier this summer
*	- all it does it print bigger versions of the ASCII character set (extended ASCII characters to be added)
*	- https://github.com/WillLillis/BigChars
*	- I'm not sure what the best way is to integrate this into this project, so for now I'll prolly 
*	just copy and paste the header files into this project's directory when I do decide to use it
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
* representing it into the avail_graphs array, and increase the NUM_GRAPH_FAMS definition
* accordingly
* 
* (TO BE IMPLEMENTED) Generated graphs will be placed in folders according to their graph
* family name
* 
*/
// have to declare (but not define) the functions up here so that they
// can be in the avail_graphs array, define below
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
//#define ADJ_DIR				"C:\\Users\\willl\\Desktop\\Adjacency Information" // (for example on my machine...)
//#define RESULT_DIR			"C:\\Users\\willl\\Desktop\\Results"

// lil helper function for taking in user inputs
bool is_number(std::string input)
{
	bool non_empty = false;
	for (char c : input) // scary modern C++ voodoo magic iterator
	{
		non_empty = true;
		if (!std::isdigit(c))
		{
			return false;
		}
	}

	return non_empty; // want to make sure an empty string wasn't passed in
}

//ASCII escape sequence magic, I don't know it just works
//https://copyprogramming.com/howto/c-how-do-i-erase-a-line-from-the-console
void erase_lines(uint_fast16_t num_lines) 
{
	if (num_lines > 0) 
	{
		printf("\x1b[2K"); // Delete current line

		for (uint_fast16_t line = 1; line < num_lines; line++) // i=1 because we included the first line
		{
			printf("\x1b[1A"); // Move cursor up one
			printf("\x1b[2K"); // Delete the entire line
		}
		printf("\r"); // Resume the cursor at beginning of line
	}
}

void print_game_results(GAME_STATE p1_result)
{
	printf("%s Wins!\n", p1_result == WIN_STATE ? "P1" : "P2");
}

// this is kind of long...look for ways to break up?
// also needs LOTS of testing....
void user_plays(std::filesystem::path adj_info_path)
{
	assert(std::filesystem::directory_entry(adj_info_path).exists());

	uint_fast16_t num_nodes = 0; // have to use malloc here?
	uint_fast16_t* adj_info = load_adjacency_info(adj_info_path, &num_nodes);
	assert(adj_info != NULL);
	assert(num_nodes > 0);

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
	std::filesystem::directory_entry dir;
	if (output_select == 1)
	{
		// change all of this to a function, i.e. verify_results_dir
#ifdef RESULT_DIR
		result_path = std::filesystem::path(RESULT_DIR);
		dir = std::filesystem::directory_entry(result_path);
#else
		result_path = std::filesystem::current_path();
		result_path.append("Results");
#endif // RESULT_DIR

		dir = std::filesystem::directory_entry(result_path);

		if (!dir.exists()) // if we can't find the directory...
		{
			printf("ERROR: Unable to find the \"Results\" directory.\n");
#ifdef RESULT_DIR // if the user supplied their own directory for their results, tell them there's something wrong with it
			printf("The alternate directory variable is defined, make sure you supplied a valid directory.\n");
			printf("Press [ENTER] to continue\n");
			char throw_away = std::getchar();
			return;
#else
			printf("The alternate directory variable is not defined, the search was completed in the project's current directory.\n"); // otherwise we'll do things in the project's current directory
			printf("Creating the necessary directory now...\n");
			if (!std::filesystem::create_directory(".\\Results")) // if the call failed to create the directory....
			{
				printf("ERROR: Failed to create the specified directory.\n");
				printf("Press [ENTER] to continue\n");
				char throw_away = std::getchar();
				return;
			}
			else // otherwise we're good (else is unecessary but helps with readability?)
			{
				printf("Press [ENTER] to continue\n");
				char throw_away = std::getchar();
			}
#endif // ADJ_DIR
		}
		// otherwise, the directory exists and we're fine to proceed
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

	GAME_STATE game_result;
	uint_fast16_t* edge_use = (uint_fast16_t*)calloc(num_nodes * num_nodes, sizeof(uint_fast16_t));
	uint_fast16_t* node_use = (uint_fast16_t*)calloc(num_nodes, sizeof(uint_fast16_t));
	assert(edge_use != NULL);
	assert(node_use != NULL);

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
		assert(move_hist != NULL);
		FILE* result_stream;
		
		std::string file_name = adj_info_path.stem().string();
		file_name.append(game_select == 0 ? " -MAC- " : " -AAC- ");
		file_name.append("SN "); 
		file_name.append(std::to_string((unsigned)node_select));
		file_name.append(".txt");
		result_path.append(file_name);
		printf("%s\n", result_path.string().c_str());
		
		errno_t err = fopen_s(&result_stream, result_path.string().c_str(), "w");
		assert(result_stream != NULL);

		if (game_select == 0) // MAC
		{
			game_result = play_MAC_loud(node_select, num_nodes, adj_info, edge_use, node_use, move_hist, 0, result_stream);
		}
		else // AAC
		{
			game_result = play_AAC_loud(node_select, num_nodes, adj_info, edge_use, node_use, move_hist, 0, result_stream);
		}

		free(move_hist);
		fclose(result_stream);
	}


	free(adj_info);
	free(edge_use);
	free(node_use);

	printf("\n\nFile: %s, Starting Node: %hhu, Game: %s\n", adj_info_path.filename().string().c_str(), node_select, game_select == 0 ? "MAC" : "AAC");
	print_game_results(game_result);

	printf("Press [ENTER] to continue\n");
	char throw_away = std::getchar();
}


// have Adjacency Info directory
	// defaults to cwd of project, but allow user to add a #define in this file
	// if they want to change that for convenience or whatever other reason
	// below code will look for said directory,
		// if it can't find it, print an error message and return
		// if it's found, list out contents
			// plan is to have folders for each graph family, but if user wants to 
			// stick raw files in that's fine
		// let user select (able to tell which is which?)
			// if a raw file is chosen, go through the normal playing steps
				// loud vs. quiet run
					// allow file naming, or do it automatically?
					// experience says just do it automatically, with game, graph name and starting node, maybe add naming option later on
					// stick results in results directory(?)
						// make default in cwd, allow re-definition in similar fashion in this file
			// if a directory is chosen, open it up and list contents
			// hard code a [BACK] option for sub-directories, 
			// [EXIT] option for basic adjacency info directory
// how do we want to pass this?
void play_menu_subdir(std::filesystem::path curr_dir)
{
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

		file_selection = entry_index + 1; //initially unacceptable value 
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

void play_menu()
{
	system("cls");
	std::filesystem::path adj_path;
#ifdef ADJ_DIR // if the user supplied their own directory for the adjacency files, use it
	adj_path = ADJ_DIR;
#else
	//adj_path = ".\\Adjacency Information"; // otherwise we'll do things in the project's current directory
	adj_path = std::filesystem::current_path(); // more portable than the above line?
	adj_path.append("Adjacency Information");
#endif // ADJ_DIR

	std::filesystem::directory_entry adj_dir(adj_path);
	if (!adj_dir.exists()) // if we can't find the directory....
	{
		printf("ERROR: Unable to find the \"Adjacency Information\" directory.\n");
#ifdef ADJ_DIR // if the user supplied their own directory for the adjacency files, tell them there's something wrong with it
		printf("The alternate directory variable is defined, make sure you supplied a valid directory.\n");
#else
		printf("The alternate directory variable is not defined, the search was completed in the project's current directory.\n"); // otherwise we'll do things in the project's current directory
		printf("Creating the necessary directory now...\n");
		std::filesystem::create_directory(".\\Adjacency Information");
#endif // ADJ_DIR
		printf("Press [ENTER] to continue\n");
		char throw_away = std::getchar();
		return;
	}

	play_menu_subdir(adj_dir);

	//// get user input, figure out how we want to handle play files or directories separately
	//	// if it's a normal file, just assume we can play with it, and let the read in funcs return errors
	//	// if it's a directory, call open_subdir func with level of recursion specified, each subsequent call increases
	//		// want to do this recursively, or is there a straightforward way to do it iteratively
	//		// once a game is played, we'll clear the screen and then have results printed to screen, 
	//		// and once [ENTER] is hit, we'll just be browsing the same directory again, can go back if you want

	//char throw_away = std::getchar();
	// should be doable with some careful string manipulation management...
	// 
	//
	// Loud and Quiet options...
	
	// figure out file system stuff here...std::filesystem?
		// just need to graph std::string of selection path
		// could be easier if we just keep things in the current directory
			// need to think about dealing with separate folders for each graph fam

	// maybe add an adjacency directory to the current directory, throw an error if it's not there
		// have code built assuming graph family folders are within that directory, and then the appropriate files within those folders
		// provide means to go into and back out of folders...
			// EXIT at adjacency directory returns from function
			// otherwise play game, then return back and chill in the same directory
}


// We'll define the some generating functions to be called by the user that will act as wrappers for the true generating functions
// These functions will attempt to open the file to write to, take in any necessary parameters from the user, 
// call the actual generating function, and then close the file after it has finished its work 
// these will also perform any necessary parameter checks 
// Organizationally these functions are in this file (instead of Adjacency_Matrix.h) because they're more of a direct result in
// how I'm implementing the menu than how the graphs are generated

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
	std::string file_name = avail_graphs[GEN_PET_ENTRY].graph_name;
	size_t add_size = num_digits(n_param) + num_digits(k_param) + 10; // param digits, 2 spaces, 2 parenthases, 1 comma, 4 from ".txt", 1 null terminator
	char* name_add = (char*)malloc(add_size);
	assert(name_add != NULL);
	snprintf(name_add, add_size, " (%hhu, %hhu).txt", n_param, k_param);
	file_name.append(name_add);
	free(name_add);
	fopen_s(&output, file_name.c_str(), "w");
	assert(output != NULL);

	generalized_petersen_gen(output, n_param, k_param); // call the actual generation function
	fclose(output);

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
	std::string file_name = avail_graphs[STACKED_PRISM_ENTRY].graph_name;
	size_t add_size = num_digits(m_param) + num_digits(n_param) + 10; // param digits, 2 spaces, 2 parenthases, 1 comma, 4 from ".txt", 1 null terminator
	char* name_add = (char*)malloc(add_size);
	assert(name_add != NULL);
	snprintf(name_add, add_size, " (%hhu, %hhu).txt", m_param, n_param);
	file_name.append(name_add);
	free(name_add);
	fopen_s(&output, file_name.c_str(), "w");
	assert(output != NULL);

	stacked_prism_gen(output, m_param, n_param); // call the actual generation function
	fclose(output);

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
	} while (!(m_param >= 3)
		|| !(n_param >= 1));

	printf("Generating graph...");
	FILE* output;
	std::string file_name = avail_graphs[Z_MN_ENTRY].graph_name;
	size_t add_size = num_digits(m_param) + num_digits(n_param) + 10; // param digits, 2 spaces, 2 parenthases, 1 comma, 4 from ".txt", 1 null terminator
	char* name_add = (char*)malloc(add_size);
	assert(name_add != NULL);
	snprintf(name_add, add_size, " (%hhu, %hhu).txt", m_param, n_param);
	file_name.append(name_add);
	free(name_add);
	fopen_s(&output, file_name.c_str(), "w");
	assert(output != NULL);

	stacked_prism_gen(output, m_param, n_param); // call the actual generation function
	fclose(output);

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
		do {
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
			if (graph_choice == NUM_GRAPH_FAMS)
			{
				return;
			}
		} while (!(graph_choice >= 0 && graph_choice < NUM_GRAPH_FAMS));

		// call appropriate function
		avail_graphs[graph_choice].gen_func();
	}
}

void main_menu()
{
	// what do we want to do with the welcome screen?
	printf("Welcome to the Cycle Games on Graphs Research Tool!\n");
	printf("This code was written and developed by <How do we wanna do names here?>\n");
	printf("Press [ENTER] to continue\n");

	int unused_char = std::getchar(); // non-implementation specific?

	std::string raw_menu_choice;
	uint_fast16_t menu_choice = 3; // initialize to unacceptable value for weird edge case
	while (true)
	{
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
				continue; // make sure this still works with the nested loops
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
