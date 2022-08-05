#include "Cycle_Games.h"
#include "Menu.h"
#include <chrono> // just for testing stuff

int main()
{
	main_menu();

	return 0;
}
// go through and add a manual if block wherever you have an assert so we can run in a release config as well
// go back and add the formal comment headers to all the functions and other stuff you defined
// start playing with multithreaded version
	// where to draw line and use single threaded version where overhead offsets benefit?