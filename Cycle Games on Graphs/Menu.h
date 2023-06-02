#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <filesystem> // Property Pages->Configuration Properties->General->C++ Language Standard->ISO C++ 17 Standard (/std:c++17) (OR NEWER)
#include <vector>
#include "Adjacency_Matrix.h"
#include "Cycle_Games.h"
#include "Cycle_Games_Threaded.h"
#include "Misc.h"
#include <chrono> // testing purposes...

/*
* TODO:
*	- look into using std::filesystem member functions more
*	- more permission checking to avoid errors
*	- https://en.cppreference.com/w/cpp/filesystem/relative to compose relative paths instead of concatenating strings
*	- check for available space before writing?
*/

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
* besides writing the generating and user generate function (takes in user input for the graph
* parameters and calls the generating function) will be to add an instance of the below struct
* representing it into the avail_graphs array, add a #define for the entry number in the
* array, and increase the NUM_GRAPH_FAMS definition accordingly
* 
*/
// have to declare (but not define) the functions up here so that they
// can be in the avail_graphs array, defined below
void user_generalized_petersen_gen();
void user_stacked_prism_gen();
void user_z_mn_gen();

// same deal with these 
void play_menu();
void generate_menu();

// Visual stido is giving me warnings that the above functions don't 
// have definitions, even though they're clearly defined farther down in the file
// the code still compiles and runs, but I'd really like to know how to fix this

typedef struct GRAPH_GEN_INFO {
	std::string graph_name{};
	std::string graph_file_name{};
	void (*gen_func)() = NULL;
	//void (*gen_func)(FILE*, uint_fast16_t, uint_fast16_t); // old way, where generating functions were called directly
}GRAPH_GEN_INFO;

// If we want to generalize the above idea for all menu choices, we'd just have to create an instance of the below struct
// for all the menus (besides the ones that vomit out file/ directory lists...), should be relatively straightforward
typedef struct MENU_ENTRY {
	std::string display_name{};
	std::string internal_name{};
	void (*select_func)() = NULL;
}MENU_ENTRY;

// menu options for main_menu() function
MENU_ENTRY main_menu_options[] = {
	MENU_ENTRY{"Play a game", "", play_menu},
	MENU_ENTRY{"Generate an adjacency listing", "", generate_menu}
};

#define NUM_MAIN_MENU_OPTIONS	2
#define MAIN_MENU_PLAY_ENTRY	0
#define MAIN_MENU_GEN_ENTRY		1

//// Generalize to a menu entry struct-> all functions must be void with no args, should be fine
//GRAPH_GEN_INFO avail_graphs[] = {
//	GRAPH_GEN_INFO{"Generalized Petersen", "Generalized_Petersen", user_generalized_petersen_gen},
//	GRAPH_GEN_INFO{"Stacked Prism", "Stacked_Prism", user_stacked_prism_gen},
//	GRAPH_GEN_INFO{"Z_m^n", "Z_m^n", user_z_mn_gen}
//};

MENU_ENTRY gen_menu_options[] = {
	MENU_ENTRY{"Generalized Petersen", "Generalized_Petersen", user_generalized_petersen_gen},
	MENU_ENTRY{"Stacked Prism", "Stacked_Prism", user_stacked_prism_gen},
	MENU_ENTRY{"Z_m^n", "Z_m^n", user_z_mn_gen}
};

// If the order in the avail_graphs array is changed or a new entry is added, these
// #define's should be updated accordingly
#define NUM_GRAPH_FAMS					3 // change name of this #define?
#define GEN_MENU_GEN_PET_ENTRY			0
#define GEN_MENU_STACKED_PRISM_ENTRY	1
#define GEN_MENU_Z_MN_ENTRY				2

// Leave commented out if you want files in the project's working directory
//#define ALT_ADJ_PATH			"C:\\Users\\willl\\Desktop\\Adjacency_Information"	// (for example on my machine...)
//#define ALT_RESULT_PATH		"C:\\Users\\willl\\Desktop\\Results"				// ^
#if defined(ALT_ADJ_PATH)
	#pragma message ("Alternate adjacency info file directory supplied: " ALT_ADJ_PATH)
#else 
	#pragma message ("Defaulting to the project's working directory for the adjacency information file directory")
#endif // ALT_ADJ_PATH
#if defined(ALT_RESULT_PATH)
	#pragma message ("Alternate results directory supplied: " ALT_RESULT_PATH)
#else 
	#pragma message ("Defaulting to the project's working directory for the results file directory")
#endif // ALT_RESULT_PATH

#define ERRNO_STRING_LEN	1000 // arbitrary max value for Microsoft's error messages, as it appears that strerrorlen_s isn't defined on my machine/ platform

/****************************************************************************
* print_game_results
*
* - Prints the winner of a game to the console
* - Lil helper function
*
* Parameters :
* - p1_result : the game state found for player 1 in the game's initial state
*
* Returns :
* - none
****************************************************************************/
void inline print_game_results(const GAME_STATE p1_result)
{
	printf("%s Wins!\n", p1_result == WIN_STATE ? "P1" : "P2");
}

/****************************************************************************
* verify_adj_info_path
*
* - Used to see if there is a valid "Adjacency_Information" directory
* - If a valid "Adjacency_Information" directory is found, its path is returned 
* by reference via the result_path parameter
* - Checks in the current directory, unless ALT_RESULT_PATH is defined, in
* which case the user supplied path is checked
* - If the user supplied path fails the check, an error is returned
* - If the directory isn't found in the current directory, one is created and
* the program is allowed to continue
* - Continues to check if there is a graph family specific subdirectory within
* the "Adjacency_Information" directory depending on the sub_dir_graph_fam
* parameter
*
* Parameters :
* - result_path : if a valid directory is found/ created, its path is passed
* out by reference using this parameter
* - fail_on_create : specifies whether to return an error (false) if the "Adjacency 
* Information" directory has to be created
* - sub_dir_graph_gam : which graph family subdirectory to search for if 
* specified. If not, there is no search past the "Adjacency_Information" 
* directory
*
* Returns :
* - bool : true is the directory was successfuly found/ made, false otherwise
****************************************************************************/
bool verify_adj_info_path(std::filesystem::path* __restrict adj_path, const bool fail_on_create, 
	const uint_fast16_t sub_dir_graph_fam = NUM_GRAPH_FAMS)
{
	std::filesystem::path adj_path_temp;
	
#ifdef ALT_ADJ_PATH // if the user supplied their own directory for the adjacency files, use it
	adj_path_temp = std::filesystem::path(ALT_ADJ_PATH);
#else // otherwise we'll do things in the project's current directory
	adj_path_temp = std::filesystem::current_path();
	adj_path_temp.append("Adjacency_Information");
#endif // ALT_ADJ_PATH

	std::filesystem::directory_entry adj_dir(adj_path_temp);

	if (!adj_dir.exists()) [[unlikely]] { // if we can't find the directory....
#ifdef ALT_ADJ_PATH // if the user supplied their own directory for the adjacency files, tell them there's something wrong with it
		DISPLAY_ERR(true,
			"Unable to find the \"Adjacency_Information\" directory.\nThe alternate path identifier is defined, make sure you supplied a valid path.\nRequested path: %s", ALT_ADJ_PATH);
		return false; // indicates an error occurred
#else
		DISPLAY_ERR(false,
			"Unable to find the \"Adjacency_Information\" directory.\nThe alternate path identifier is not defined, the search was completed in the project's current directory.");
		printf("Creating the necessary directory now...\n");
		if (!std::filesystem::create_directory(adj_path_temp)) {
			DISPLAY_ERR(true,
				"Failed to create the specified \"Adjacency_Information\" directory\nRequested path: %s", adj_path_temp.string().c_str());
			
			return false;
		}
		printf("Done.\nPress [ENTER] to continue...\n");
		char throw_away = std::getchar();
		return !fail_on_create; // if fail_on_create is true, we need to return false
#endif // ALT_ADJ_PATH
		// there's no reason to continue past this point, beacuse if the "Adjacency_Information" directory wasn't found, then there's no files that can be loaded and played on
	}

	// otherwise the Adjacency_Information directory exists...now it's time to check for graph family-specific sub directories
	std::filesystem::path graph_fam_subpath;
	if (sub_dir_graph_fam >= 0 && sub_dir_graph_fam < NUM_GRAPH_FAMS) { // if the specified graph family parameter is valid
		graph_fam_subpath = adj_path_temp;
		graph_fam_subpath.append(gen_menu_options[sub_dir_graph_fam].internal_name);
		std::filesystem::directory_entry graph_fam_subdir(graph_fam_subpath);
		if (graph_fam_subdir.exists()) {
			*adj_path = graph_fam_subpath;
			return true;
		}
	}
	
	// if it's invalid or if the graph family's folder doesn't exist, just stick the file in the Adjacency_Information directory
	*adj_path = adj_path_temp;
	return true;
}

/****************************************************************************
* verify_results_path
*
* - Used to see if there is a valid "Results" directory 
* - If a valid "Results" directory is found, its path is returned by 
* reference via the result_path parameter
* - Checks in the current directory, unless ALT_RESULT_PATH is defined, in 
* which case the user supplied path is checked
* - If the user supplied path fails the check, an error is returned
* - If the directory isn't found in the current directory, one is created and 
* the program is allowed to continue
*
* Parameters :
* - result_path : if a valid directory is found/ created, its path is passed
* out by reference using this parameter
* - fail_on_create : specifies whether to return an error if the "Results" 
* directory has to be created
*
* Returns :
* - bool : true is the directory was successfuly found/ made, false otherwise
****************************************************************************/
bool verify_results_path(std::filesystem::path* __restrict result_path, const bool fail_on_create)
{
	std::filesystem::path result_path_temp;

#ifdef ALT_RESULT_PATH
	result_path_temp = std::filesystem::path(ALT_RESULT_PATH);
#else
	result_path_temp = std::filesystem::current_path();
	result_path_temp.append("Results");
#endif // ALT_RESULT_PATH

	std::filesystem::directory_entry result_dir(result_path_temp);

	if (!result_dir.exists()) [[unlikely]] { // if we can't find the directory...
#ifdef ALT_RESULT_PATH // if the user supplied their own directory for their results, tell them there's something wrong with it
		DISPLAY_ERR(true,
			"Unable to find the \"Results\" directory.\nThe alternate path identifier is defined, make sure you supplied a valid path.\n Requested path: %s", result_path_temp.string().c_str());
		return false;
#else
		printf("WARNING: Unable to find the \"Results\" directory.\n");
		printf("The alternate path identifier is not defined, the search was completed in the project's current directory.\n"); // otherwise we'll do things in the project's current directory
		printf("Creating the necessary directory now...\n");
		if (!std::filesystem::create_directory(result_path_temp)) { // if the call failed to create the directory....
			DISPLAY_ERR(false,
				"Failed to create the specified \"Results\" directory\nRequested path: %s", result_path_temp.string().c_str());
			return false;
		} else { // otherwise we're good 
			*result_path = result_path_temp;
			printf("Done.\nPress [ENTER] to continue\n");
			char throw_away = std::getchar();
			return !fail_on_create;
		}
#endif // ALT_RESULT_PATH
	}
	// otherwise, the directory exists and we're fine to proceed
	*result_path = result_path_temp;
	return true;
}

/****************************************************************************
* get_result_file_name
*
* - Takes in a path to an adjacency info file and some other related
* information, returns a file name for a result file
*
* Parameters :
* - adj_info_path : path to the adjacency info file being used
* - game_select : indicates what game is being played
*	- 0 for MAC
*	- 1 for AAC
*	- do we want to do an enum for this instead of just arbitrary int values?
* - starting_node : the node number the game is starting on
*
* Returns :
* - std::string : the generated name for the specified game's result file
****************************************************************************/
std::string get_result_file_name(const std::filesystem::path adj_info_path,
	const uint_fast16_t game_select, const uint_fast16_t starting_node)
{
	std::string file_name = adj_info_path.stem().string(); // want a check here to make sure there's a stem?
	file_name.append(game_select == 0 ? "-MAC-" : "-AAC-");
	file_name.append(std::to_string(starting_node));
	file_name.append(".txt");

	return file_name;
}

/****************************************************************************
* user_plays
*
* - Function called when the user elects to play a game on a specified
* adjacency information file
* - Prompts the user for 
*	- MAC or AAC
*	- the starting node
*	- quiet or loud run
* - Displays the game's result after completion
*
* Parameters :
* - adj_info_path : path to the adjacency information file to be played on
*
* Returns :
* - none
****************************************************************************/
// this is kind of long...look for ways to break up?
// want to change [BACK] options to go back a step in param selection, instead of back to the file selection page?
void user_plays(const std::filesystem::path adj_info_path)
{
	if (!std::filesystem::directory_entry(adj_info_path).exists()) [[unlikely]] {
		DISPLAY_ERR(true,
			"Supplied adjacency info file was not found/does not exist\nRequested path: %s", adj_info_path.string().c_str());
		return;
	}

	uint_fast16_t num_nodes = 0; 
	bool load_success = false;
	std::vector<uint_fast16_t> adj_info = load_adjacency_info(adj_info_path, &num_nodes, &load_success); // call returns pointer to the adjacency matrix, and sets the value of num_nodes
	if (load_success == false) [[unlikely]] {
		DISPLAY_ERR(true,
			"An error occurred while attempting to load adjacency information");
		return;
	}
	if (!(num_nodes > 0)) [[unlikely]] {
		DISPLAY_ERR(true,
			"Recieved invalid graph parameter (number of graphs nodes) after attempting to load adjacency information. Value: %hu", (uint16_t)num_nodes);
		return;
	}

	// prompt user for game (MAC or AAC)
	bool bad_input = false;
	std::string game_select_raw;
	uint_fast16_t game_select = 3;
	clear_screen();
	printf("Select the game to play:\n");
	printf("[0] MAC\n"); // change to "Make-A-Cycle (MAC)" ??
	printf("[1] AAC\n"); // change to "Avoid-A-Cycle (AAC)" ??
	//printf("[2] GG\n"); // GG for Generalized Geography?
	printf("[2] [BACK]\n"); // would have to change to [3]-> want to have a NUM_GAMES define somewhere maybe-> could do a similar struct/ array combo as with the graph families...
	do {
		if (bad_input) {
			erase_lines(2);
		}
		bad_input = true;
		std::cin >> game_select_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(game_select_raw)) {
			continue;
		}
		game_select = std::stoul(game_select_raw, NULL);
		if (game_select == 2) { // [BACK] option
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
	do {
		if (bad_input) {
			erase_lines(2);
		}
		bad_input = true;
		std::cin >> output_select_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(output_select_raw)) {
			continue;
		}
		output_select = std::stoul(output_select_raw, NULL);
		if (output_select == 2) { // [BACK] option
			return;
		}
	} while (!(output_select >= 0 && output_select <= 2));

	// if the user asked for a loud run, make sure the results directory is all set up
	std::filesystem::path result_path;
	if (output_select == 1) {
		if (!verify_results_path(&result_path, false)) [[unlikely]] {
			DISPLAY_ERR(true,
				"Failed to find and/ or create the \"Results\" sub-directory to store the results of the loud run.");
			return;
		}
	}

	// prompt user for starting node on graph
	bad_input = false;
	std::string node_select_raw;
	uint_fast16_t node_select = num_nodes;
	printf("Select the starting node:\n");
	printf("[0 - %hu] Said node\n", (uint16_t)(num_nodes - 1));
	printf("[%hu] [BACK]\n", (uint16_t)num_nodes);
	do {
		if (bad_input) {
			erase_lines(2);
		}
		bad_input = true;
		std::cin >> node_select_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(node_select_raw)) [[unlikely]] {
			continue;
		}
		node_select = std::stoul(node_select_raw, NULL);
		if (node_select == num_nodes) { // [BACK] option
			return;
		}
	} while (!(node_select >= 0 && node_select < num_nodes));

	// Now that all of the options have been specified, it's time to set up to actually play the game as requested
	std::vector<uint_fast16_t> edge_use(num_nodes * num_nodes, NOT_USED);
	std::vector<uint_fast16_t> node_use(num_nodes, NOT_USED);

	// Have to mark the starting node as used!
	node_use[node_select] = USED;

	GAME_STATE game_result;
	if (output_select == 0) { // Quiet 
		if (game_select == 0) { // MAC
			game_result = play_MAC_quiet(node_select, num_nodes, adj_info, edge_use, node_use);
		} else { // AAC
			game_result = play_AAC_quiet(node_select, num_nodes, adj_info, edge_use, node_use);
		}
	}
	else { // Loud
		std::vector<uint_fast16_t> move_hist(num_nodes);

		FILE* result_stream;
		std::string file_name = get_result_file_name(adj_info_path, game_select, node_select);
		result_path.append(file_name);
#ifdef _WIN32 // might as well use Microsoft's error reporting if we're on a windows machine
		_set_errno(0); // "Always clear errno by calling _set_errno(0) immediately before a call that may set it"
		errno_t err = fopen_s(&result_stream, result_path.string().c_str(), "w"); 
#else
		result_stream = fopen(result_path.string().c_str(), "w");
		//fopen_s(&result_stream, result_path.string().c_str(), "w");
#endif // _WIN32

#ifdef _WIN32
		if (err != 0) [[unlikely]] {
			char err_buff[ERRNO_STRING_LEN]; // (https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-s-strerror-s-wcserror-s-wcserror-s?view=msvc-170)
			strerror_s(err_buff, ERRNO_STRING_LEN, NULL);
			DISPLAY_ERR(true,
				"Failed to open the result output file.\nRequested path: %s\nfopen_s error message: %s", result_path.string().c_str(), err_buff);
#else
		if (result_stream == NULL) [[unlikely]] {
			DISPLAY_ERR(true,
				"Failed to open the result output file.\nRequested path: %s", result_path.string().c_str());
#endif // _WIN32
			return;
		}
		if (game_select == 0) { // MAC
			game_result = play_MAC_loud(node_select, num_nodes, adj_info, edge_use, node_use, move_hist, 0, result_stream);
		} else { // AAC
			game_result = play_AAC_loud(node_select, num_nodes, adj_info, edge_use, node_use, move_hist, 0, result_stream);
		}

		if (result_stream != NULL) [[likely]] { // if result_stream is NULL, then we don't need to close it
			int close_err = fclose(result_stream);
			if (close_err != 0) [[unlikely]] {
				DISPLAY_ERR(false,
					"Failed to properly close the output file.\nPath associated with file stream: %s", result_path.string().c_str());
			}
		}
	}
	
	printf("\n\nFile: %s, Starting Node: %hu, Game: %s\n", adj_info_path.filename().string().c_str(), (uint16_t)node_select, game_select == 0 ? "MAC" : "AAC");
	print_game_results(game_result);

	printf("Press [ENTER] to continue\n");
	char throw_away = std::getchar();
}

/****************************************************************************
* play_menu_subdir
*
* - Allows the user to browse in a sub-directory of "Adjacency_Information"
* in order to look for a file to play a game on
* - Calls itself recursively to open additionally nested sub-directories
*
* Parameters :
* - curr_dir : the current directory that this call of the function will browse 
* within
*
* Returns :
* - none
****************************************************************************/
void play_menu_subdir(const std::filesystem::path curr_dir)
{
	if (!std::filesystem::directory_entry(curr_dir).exists()) [[unlikely]] {
		DISPLAY_ERR(true,
			"Failed to find the specified subdirectory.\nRequested path: %s", curr_dir.string().c_str());
		return;
	}

	uint_fast16_t entry_index = 0;
	std::filesystem::path temp_path = curr_dir;
	bool bad_selection = false;
	std::string file_selection_raw;
	uint_fast16_t file_selection;

	while (true) {
		clear_screen();
		// is having the full path displayed helpful?
		printf("%s\n", curr_dir.string().c_str()); // print the current directory so the user knows where they are
		printf("Select a [FILE] to play on, or a [DIR] to open.\n");
		entry_index = 0;

		for (auto const& dir_entry : std::filesystem::directory_iterator(curr_dir)) {
			if (dir_entry.is_directory()) {
				printf("[%03hu][DIR]  %s\n", (uint16_t)entry_index, dir_entry.path().filename().string().c_str());
				entry_index++;
			} else if (dir_entry.is_regular_file()) {
				printf("[%03hu][FILE] %s\n", (uint16_t)entry_index, dir_entry.path().filename().string().c_str());
				entry_index++;
			}
		}
		if (entry_index == 0) [[unlikely]] {
			printf("<No valid entries>\n");
		}
		printf("[%03hu][BACK]\n", (uint16_t)entry_index);

		file_selection = entry_index + 1; // initially unacceptable value 
		bad_selection = false;
		do {
			if (bad_selection) {
				erase_lines(2); // need to erase 2 lines because hitting enter puts us on a newline, and then we have to erase the original bad line
			}
			bad_selection = true;
			std::cin >> file_selection_raw;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			if (!is_number(file_selection_raw)) {
				continue;
			}
			file_selection = std::stoul(file_selection_raw, NULL);
			if (file_selection == entry_index) { // [BACK] option
				return;
			}
		} while (!(file_selection >= 0 && file_selection < entry_index));

		// once we're past that loop, the file_selection value represents a file or directory to open...
		// reading through the C++ documentation, I didn't find a way to directly access a member of the directory
		// using the std::filesystem functions-> guess we'll just iterate through until we get to the right one
			// there has to be a better way to do this, right?
			// could append all the dir entries to a std::vector and grab from that...
		entry_index = 0;
		temp_path = curr_dir;
		for (auto const& dir_entry : std::filesystem::directory_iterator(curr_dir)) { // starts in cwd
			if (dir_entry.is_directory()) {
				if (entry_index == file_selection) {
					temp_path /= dir_entry.path().filename();
					play_menu_subdir(temp_path);
					break;
				} else {
					entry_index++;
				}
			} else if (dir_entry.is_regular_file()) {
				if (entry_index == file_selection) {
					user_plays(dir_entry.path());
					break;
				} else {
					entry_index++;
				}
			}
		}
	}
}

/****************************************************************************
* play_menu
*
* - Function called when the user elects to play a game in the main menu
* - Does some checks on the adjacency information files, before then calling 
* play_menu_subdir to allow the user to browse through all of the adjacency 
* information files
*
* Parameters :
* - none
*
* Returns :
* - none
****************************************************************************/
void play_menu()
{
	clear_screen();
	std::filesystem::path adj_path;

	if (!verify_adj_info_path(&adj_path, true)) [[unlikely]] { // make sure the adjacency info directory is there
		DISPLAY_ERR(true,
			"Issues with/ couldn't find the \"Adjacency_Information\" directory.\nReturned path: %s", adj_path.string().c_str());
		return;
	}

	play_menu_subdir(adj_path); // and if so then start browsing
}

/****************************************************************************
* get_adj_info_file_name
*
* - Takes in a graph family and its associated graph parameters and returns a 
* std::string of the name for the graph's adjacency information file
* - Name is of the form "<Graph Family Name> (param1, param2, ...).txt"
* - Need to be able to walk through a variable number of graph parameters...
* think it's safe to assume they're all uint_fast16_t's, can make a change 
* later if it's needed
*
* Parameters :
* - graph_fam : Which graph family the name is being made for
* - num_args : the number of optional graph parameters to follow
*	- uint_fast32_t type chosen for compatability with va_start macro,
*	as arguments undergo promotion and behavior can be undefined
* - ... : variable number of optional arguments corresponding to graph 
* parameters
*
* Returns :
* - std::string : the generated name for the specified graph's adjacency 
* information file
****************************************************************************/
// safer way to do va_args than the C way?
std::string get_adj_info_file_name(const uint_fast16_t graph_fam, const uint_fast32_t num_args, ...)
{
	if (!(graph_fam >= 0 && graph_fam < NUM_GRAPH_FAMS)) [[unlikely]] {
		DISPLAY_ERR(true,
			"Invalid graph family parameter supplied to generate the file name.");
		return "";
	}

	std::string file_name = gen_menu_options[graph_fam].internal_name; // false positive warning pops up here
	if (num_args == 0) [[unlikely]] { // no graph parameters
	
		file_name.append(".txt");
		return file_name;
	}

	file_name.append("_(");

	va_list arg_ptr;
	va_start(arg_ptr, num_args);
	for (uint_fast16_t curr_arg = 0; curr_arg < num_args; curr_arg++) {
		if (curr_arg > 0) {
			file_name.append(",");
		}
#ifdef WIN32 
		file_name.append(std::to_string(va_arg(arg_ptr, uint_fast16_t)));
#else
		// gcc does NOT like you passing uint_fast16_t to va_arg, we'll try this (issue seems to be integer promotion)
		// didn't see a straightforward way with C++ to do variable arguments, so we'll just go with this for now
		file_name.append(std::to_string(va_arg(arg_ptr, int)));
#endif // WIN32
	}
	va_end(arg_ptr);
	file_name.append(").txt");
	
	return file_name;
}

// We'll define the some generating functions to be called by the user that will act as wrappers for the true generating functions
// These functions will attempt to open the file to write to, take in any necessary parameters from the user, 
// call the actual generating function, and then close the file after it has finished its work 
// these will also perform any necessary parameter checks 
// Organizationally these functions are in this file (instead of Adjacency_Matrix.h) because they're more of a direct result of
// how I'm implementing the menu than how the graphs are generated

// If a subfolder of the correct name doesn't exist, just stick it in the directory-> leave it up to the user where they want their files
// If a subfolder of the correct name does exist, put it in there->organization done for those who don't care
	// do we want to add option to play immediately on a file we just generated?->talk with Gates/ Kelvey

// Might want to re-organize here, a lot of redundant code
	// maybe create separate parameter checking functions for each graph family...

/****************************************************************************
* user_generalized_petersen_gen
*
* - Function called when the user elects to generate an adjacency information
* file for the generalized petersen graph family
* - Prompts the user for the relevant graph parameters (m and n) before then
* calling the actual generation function
*
* Parameters :
* - none
*
* Returns :
* - none
****************************************************************************/
void user_generalized_petersen_gen()
{
	// Generalized Petersen graphs need 'n' and 'k' parameters to be constructed
	uint_fast16_t n_param = 0; // providing initial assignment to get uninitialized local variable warning to go away
	uint_fast16_t k_param = 0; // ^
	std::string n_raw, k_raw;

	do {
		clear_screen();
		printf("Provide the following parameters for the construction of the %s graph.\n",
			gen_menu_options[GEN_MENU_GEN_PET_ENTRY].display_name.c_str());

		printf("(The number of nodes in each of the graph's two \"rings\")\n");
		printf("n: ");
		std::cin >> n_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(n_raw)) {
			continue;
		}
		n_param = std::stoul(n_raw, NULL);

		printf("(Distance between connections for the inner ring's nodes)\n");
		printf("k: ");
		std::cin >> k_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(k_raw)) {
			continue;
		}
		k_param = std::stoul(k_raw, NULL);
	} while (!(n_param >= 3) 
		|| !(k_param >= 1) 
		|| !(k_param <= ((n_param - 1) / 2)));
	
	printf("Generating graph...");
	FILE* output;

	std::filesystem::path output_path;
	if (!verify_adj_info_path(&output_path, false, GEN_MENU_GEN_PET_ENTRY)) [[unlikely]] {
		DISPLAY_ERR(true,
			"Issue found when checking the adjacency information path.");
		return;
	}

	std::string file_name = get_adj_info_file_name(GEN_MENU_GEN_PET_ENTRY, 2, n_param, k_param);
	output_path.append(file_name);

	// can check if file exists here if we want to do some kind of versioning....

#ifdef _WIN32 // might as well use Microsoft's error reporting if we're on a windows machine
	_set_errno(0); // "Always clear errno by calling _set_errno(0) immediately before a call that may set it"
	errno_t err = fopen_s(&output, output_path.string().c_str(), "w");
#else 
	output = fopen(output_path.string().c_str(), "w");
#endif // WIN32

#ifdef _WIN32
	if (err != 0) [[unlikely]] {
		char err_buff[ERRNO_STRING_LEN]; // (https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-s-strerror-s-wcserror-s-wcserror-s?view=msvc-170)
		strerror_s(err_buff, ERRNO_STRING_LEN, NULL);
		DISPLAY_ERR(true,
			"Failed to open the result output file.\nRequested path: %s\nfopen_s error message: %s", 
			output_path.string().c_str(), err_buff);
#else
	if (output == NULL) [[unlikely]] {
		DISPLAY_ERR(true,
			"Failed to open the result output file.\nRequested path: %s", output_path.string().c_str());
#endif // WIN32
		return;
	}

	generalized_petersen_gen(output, n_param, k_param); // call the actual generation function
	if (output != NULL) [[likely]] {
		int close_err = fclose(output);
		if (close_err != 0) [[unlikely]] {
			DISPLAY_ERR(true,
				"Failed to properly close the output file.\nPath associated with file stream: %s", 
				output_path.string().c_str());
			return;
		}
	}

	// allow user to play game on newly generated adjacency file
	std::string exit_choice_raw;
	uint_fast16_t exit_choice = 2;
	bool bad_input = false;
	do {
		if (!bad_input) {
			printf("Generation completed.\n");
			printf("[0] Continue\n");
			printf("[1] Play a game on %s\n", output_path.stem().string().c_str());
		} else {
			erase_lines(2);
		}
		std::cin >> exit_choice_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		bad_input = true;
		if (!is_number(exit_choice_raw)) {
			continue;
		}
		exit_choice = std::stoul(exit_choice_raw, NULL);
	} while (!(exit_choice >= 0 && exit_choice <= 1));

	if (exit_choice == 1) {
		user_plays(output_path);
	}
}

/****************************************************************************
* user_stacked_prism_gen
*
* - Function called when the user elects to generate an adjacency information
* file for the stacked prism graph family
* - Prompts the user for the relevant graph parameters (m and n) before then
* calling the actual generation function
*
* Parameters :
* - none
*
* Returns :
* - none
****************************************************************************/
void user_stacked_prism_gen()
{
	// Stacked Prism graphs need 'm' and 'n' parameters to be contructed
	uint_fast16_t m_param = 0; // providing initial assignment to get uninitialized local variable warning to go away
	uint_fast16_t n_param = 0; // ^
	std::string m_raw, n_raw;

	do {
		clear_screen();
		printf("Provide the following parameters for the construction of the %s graph.\n",
			gen_menu_options[GEN_MENU_GEN_PET_ENTRY].display_name.c_str());

		printf("(The number of nodes in each concentric ring)\n");
		printf("m: ");
		std::cin >> m_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(m_raw)) {
			continue;
		}
		m_param = std::stoul(m_raw, NULL);

		printf("(The number of concentric rings)\n");
		printf("n: ");
		std::cin >> n_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(n_raw)) {
			continue;
		}
		n_param = std::stoul(n_raw, NULL);
	} while (!(m_param >= 3)
		|| !(n_param >= 1));

	printf("Generating graph...");
	FILE* output;

	std::filesystem::path output_path;
	if (!verify_adj_info_path(&output_path, false, GEN_MENU_STACKED_PRISM_ENTRY)) [[unlikely]] {
		DISPLAY_ERR(true,
			"Issue found when checking the adjacency information path.");
		return;
	}

	std::string file_name = get_adj_info_file_name(GEN_MENU_STACKED_PRISM_ENTRY, 2, m_param, n_param);
	output_path.append(file_name);

#ifdef _WIN32 // might as well use Microsoft's error reporting if we're on a windows machine
	_set_errno(0); // "Always clear errno by calling _set_errno(0) immediately before a call that may set it"
	errno_t err = fopen_s(&output, output_path.string().c_str(), "w");
#else
	output = fopen(output_path.string().c_str(), "w");
#endif // WIN32
	
#ifdef _WIN32
	if (err != 0) [[unlikely]] { // Will need to tweak error reporting once we get the generated graphs in the correct directory
		char err_buff[ERRNO_STRING_LEN]; // (https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-s-strerror-s-wcserror-s-wcserror-s?view=msvc-170)
		strerror_s(err_buff, ERRNO_STRING_LEN, NULL);
		DISPLAY_ERR(true,
			"Failed to open the result output file.\nRequested path: %s\nfopen_s error message: %s", output_path.string().c_str(), err_buff);
#else
	if(output == NULL) [[unlikely]] {
		DISPLAY_ERR(true,
			"Failed to open the result output file.\nRequested path: %s\nfopen_s error code: %d", output_path.string().c_str());
#endif // _WIN32
		return;
	}

	stacked_prism_gen(output, m_param, n_param); // call the actual generation function

	if (output != NULL) [[likely]] {
		int close_err = fclose(output);
		if (close_err != 0) [[unlikely]] {
			DISPLAY_ERR(true,
				"Failed to properly close the output file.\nPath associated with file stream: %s", 
				output_path.string().c_str());
			return;
		}
	}

	// allow user to play game on newly generated adjacency file
	std::string exit_choice_raw;
	uint_fast16_t exit_choice = 2;
	bool bad_input = false;
	do {
		if (!bad_input) {
			printf("Generation completed.\n");
			printf("[0] Continue\n");
			printf("[1] Play a game on %s\n", output_path.stem().string().c_str());
		} else {
			erase_lines(2);
		}
		std::cin >> exit_choice_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		bad_input = true;
		if (!is_number(exit_choice_raw)) {
			continue;
		}
		exit_choice = std::stoul(exit_choice_raw, NULL);
	} while (!(exit_choice >= 0 && exit_choice <= 1));

	if (exit_choice == 1) {
		user_plays(output_path);
	}
}

/****************************************************************************
* user_z_mn_gen
*
* - Function called when the user elects to generate an adjacency information
* file for the Z_m^n graph family
* - Prompts the user for the relevant graph parameters (m and n) before then 
* calling the actual generation function
*
* Parameters :
* - none
*
* Returns :
* - none
****************************************************************************/
void user_z_mn_gen()
{
	// Z_m^n graphs need 'm' and 'n' parameters to be contructed
	uint_fast16_t m_param = 0; // providing initial assignment to get uninitialized local variable warning to go away
	uint_fast16_t n_param = 0; // ^
	std::string m_raw, n_raw;

	do {
		clear_screen();
		printf("Provide the following parameters for the construction of the %s graph.\n",
			gen_menu_options[GEN_MENU_Z_MN_ENTRY].display_name.c_str());

		printf("(Tuple entries ranging from 0 to m-1)\n");
		printf("m: ");
		std::cin >> m_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(m_raw)) {
			continue;
		}
		m_param = std::stoul(m_raw, NULL);

		printf("(n entries in the tuple)\n");
		printf("n: ");
		std::cin >> n_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!is_number(n_raw)) {
			continue;
		}
		n_param = std::stoul(n_raw, NULL);
	} while (!(m_param >= 2) // is this the correct constraint?
		|| !(n_param >= 1));

	printf("Generating graph...");
	FILE* output;

	std::filesystem::path output_path;
	if (!verify_adj_info_path(&output_path, false, GEN_MENU_Z_MN_ENTRY)) [[unlikely]] {
		DISPLAY_ERR(true,
			"Issue found when checking the adjacency information path.");
		return;
	}

	std::string file_name = get_adj_info_file_name(GEN_MENU_Z_MN_ENTRY, 2, m_param, n_param);
	output_path.append(file_name);
	

#ifdef _WIN32 // might as well use Microsoft's error reporting if we're on a windows machine
	_set_errno(0); // "Always clear errno by calling _set_errno(0) immediately before a call that may set it"
	errno_t err = fopen_s(&output, output_path.string().c_str(), "w");
#else
	output = fopen(output_path.string().c_str(), "w");
#endif // WIN32

#ifdef _WIN32
	if (err != 0) [[unlikely]] {
		//strerrorlen_s(err); // looks like this isn't defined on my machine :(
		char err_buff[ERRNO_STRING_LEN]; // (https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-s-strerror-s-wcserror-s-wcserror-s?view=msvc-170)
		strerror_s(err_buff, ERRNO_STRING_LEN, NULL);
		DISPLAY_ERR(true,
			"Failed to open the result output file.\nRequested path: %s\nfopen_s error message: %s", output_path.string().c_str(), err_buff);
#else
	if(output == NULL) [[unlikely]] {
		DISPLAY_ERR(true,
			"Failed to open the result output file.\nRequested path: %s", output_path.string().c_str());
#endif // _WIN32
		return;
	}

	bool success = z_mn_gen(output, m_param, n_param); // call the actual generation function
	
	if (output != NULL) [[likely]] {
		int close_err = fclose(output);
		if (close_err != 0) [[unlikely]] {
			DISPLAY_ERR(true,
				"Failed to properly close the output file.\nPath associated with file stream: %s", 
				output_path.string().c_str());
			return;
		}
	}

	if (!success) [[unlikely]] {
		DISPLAY_ERR(false, "Failed to generate adjacency matrix! Returning...");
		return;
	}

	// allow user to play game on newly generated adjacency file
	std::string exit_choice_raw;
	uint_fast16_t exit_choice = 2;
	bool bad_input = false;
	do {
		if (!bad_input) {
			printf("Generation completed.\n");
			printf("[0] Continue\n");
			printf("[1] Play a game on %s\n", output_path.stem().string().c_str());
		} else {
			erase_lines(2);
		}
		std::cin >> exit_choice_raw;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		bad_input = true;
		if (!is_number(exit_choice_raw)) {
			continue;
		}
		exit_choice = std::stoul(exit_choice_raw, NULL);
	} while (!(exit_choice >= 0 && exit_choice <= 1));

	if (exit_choice == 1) {
		user_plays(output_path);
	}
}

/****************************************************************************
* generate_menu
*
* - Function called when the user elects to generate adjacency information
* from the main menu
* - Displays the available graph families to generate, and then calls the
* appropriate user generate function once a valid selection is made
*
* Parameters :
* - none
*
* Returns :
* - none
****************************************************************************/
void generate_menu()
{
	std::string graph_choice_raw; // could just use a char since we have less than 10 choices, but this allows for easier expansion in the future
	uint_fast16_t graph_choice = NUM_GRAPH_FAMS + 1; // initialize to unacceptable value for weird edge case
	
	while (true) {
		graph_choice = NUM_GRAPH_FAMS + 1;
		do {
			clear_screen();
			printf("Select which type of graph you'd like to generate an adjacency listing for.\n");
			for (uint_fast16_t curr_choice = 0; curr_choice < NUM_GRAPH_FAMS; curr_choice++) {
				printf("[%hu] %s\n", (uint16_t)curr_choice, gen_menu_options[curr_choice].display_name.c_str());
			}
			printf("[%u] BACK\n", NUM_GRAPH_FAMS);

			std::cin >> graph_choice_raw;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			if (!is_number(graph_choice_raw)) {
				continue;
			}
			graph_choice = std::stoul(graph_choice_raw, NULL);
			if (graph_choice == NUM_GRAPH_FAMS) { // BACK option
				return;
			}
		} while (!(graph_choice >= 0 && graph_choice < NUM_GRAPH_FAMS));

		// call appropriate generating function
		gen_menu_options[graph_choice].select_func();
	}
}

/****************************************************************************
* main_menu
*
* - First function called to beging the program
* - Allows the user to choose between playing a game, generating a file 
* containing adjacency information, or to exit the program
*
* Parameters :
* - none
*
* Returns :
* - none
****************************************************************************/
// struct array for menu options here as well?
void main_menu()
{
	// what do we want to do with the welcome screen?
	printf("Welcome to the Cycle Games on Graphs Research Tool!\n");
	printf("This code was written and developed by Will Lillis (wlillis@umass.edu)\n");
	printf("Press [ENTER] to continue\n");

	char throw_away = std::getchar(); 

	std::string menu_choice_raw;
	uint_fast16_t menu_choice = 3; // initialize to unacceptable value
	while (true) {
		menu_choice = NUM_MAIN_MENU_OPTIONS + 1; // have to reset this value for new pass
		do {
			clear_screen();
			for (uint_fast16_t curr_choice = 0; curr_choice < NUM_MAIN_MENU_OPTIONS; curr_choice++) {
				printf("[%hu] %s\n", (uint16_t)curr_choice, main_menu_options[curr_choice].display_name.c_str());
			}
			printf("[%hu] EXIT\n", (uint16_t)NUM_MAIN_MENU_OPTIONS);

			std::cin >> menu_choice_raw;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			if (!is_number(menu_choice_raw)) {
				continue; 
			}
			menu_choice = std::stoul(menu_choice_raw, NULL);
			if (menu_choice == NUM_MAIN_MENU_OPTIONS) {
				return;
			}
		} while (!(menu_choice >= 0 && menu_choice < NUM_MAIN_MENU_OPTIONS));

		main_menu_options[menu_choice].select_func();
	}
}
