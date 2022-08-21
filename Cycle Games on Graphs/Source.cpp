#include "Menu.h"
#include "Cycle_Games_Threaded.h"
//#include <chrono> // just for testing stuff

int main()
{	
	main_menu();

	return 0;
}
// make user generated files save in the correct directory, or in adjacency information base directory as a default
// go through and add a manual if block wherever you have an assert so we can run in a release config as well
// go back and add the formal comment headers to all the functions and other stuff you defined
// start playing with multithreaded version
	// where to draw line and use single threaded version where overhead offsets benefit?
// Need to add ability to read in adjacency matrices, as well as regenerate some of the Z_m^n files
// want to look into prototyping the multi-threaded version
// Code to play Generalized Geography game?

// should the pointer to the adjacency matrix be marked const in the game playing function?

// Need to test Microsoft compiler on non-Windows computer->may have issues with use of #ifdef WIN32/ #ifdef _WIN32-> just need to test things out

// Add build instructions, other things to README