#include "Menu.h"
//#include "Cycle_Games_Threaded.h"

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

// Add build instructions, other things to README
	// Instructions on adding graph families to generate menu, and menu options in general with the struct

// could look at redo-ing menu UI
	// allow arrow pointer selection of menu items
	// would be a pain with lots of items aka scrolling, redrawing may run into issues
	// something to look into

// NEW PLAN!!!!!!!!!!!!!!!!!!!!!!
	// Need to rethink the multithreaded version of the game playing code
		// the thread needs to be created the THREAD_GAME_INFO's constructor, 
			// because std::thread doesn't support copying, and also doesn't allow me to keep a reference of the thread 
			// the basic idea should be the same, with a bit of the thread management stuff getting swapped out for a vector, with entries getting added on and popped up as the game operates