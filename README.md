# Cycle Games on Graphs

Computer code for playing the combinatorial games <i>Make-A-Cycle</i> (MAC) and <i>Avoid-A-Cycle</i> (AAC). These games were first defined for Cayley graphs in the paper <a href="http://dx.doi.org/10.1515/9783110755411-011">Relator Games on Groups</a>. A second paper discussing the games MAC and AAC is forthcoming (put pre-print link here when ready).

## Compiling/ Building

Download the folder <b>Cycle Games on Graphs</b>. The project can be built by compiling ``Source.cpp`` using at least ``C++20``.  

The project can be built with the provided <b>Makefile<b>, or simply with ```g++ --std=c++20 Source.cpp```. Optimizations have been added to the ``Makefile`` for performance reasons.

## Notes

Due to the difference in newline characters between Windows, Mac, and Linux, the provided adjacency files may not work on your machine. This will become evident when you attempt to use a file to simulate a game and received an error while parsing it. Newly generated adjacency information files no longer use '\n' as a delimiter. Furthermore, automatic "repair" functionality has been added where a "dos2unix"-like action is made to a copy of the file in question. If the issue persists, try re-generating from scratch.

This project originally just required ``C++17`` (due to its usage of std::filesystem) but now requires ``C++20`` due to its usage of the ``__VA_OPT__`` functional macro. I believe this means g++ version 10.0 or newer is now needed to compile the project. While I don't anticipate this being an issue, if it proves to be I can make some compatability changes in the code that will allow it to be built (albeit with less informative error reporting at runtime). 
  
An attempt was made to multithread the code, and this can still be seen in ``Cycle_Games_Threaded.h``. Unfortunately, the memory overhead of providing a private copy of the ``node_use_list`` and ``edge_use_matrix`` vectors to each job in the queue causes the program to crash even while working on moderately sized graphs.

### Adding a New Graph Family

If one wishes to add a new graph family to the list of generate-able families, the following steps can be followed: 

- Write a ``user_x_gen()`` function, and declare it at the top of ``Menu.h`` along with the others. This function should prompt the user for the relevant graph parameters, and do some common sense input checking/cleaning. The other ``user_x_gen()`` functions should be good examples for this.
- Write an ``x_gen()`` function that takes in the parameters from the first function and writes the adjacency information to a file. The other ``x_gen()`` functions should be also good examples for this.
- Add an entry for the graph family in the ``gen_menu_options`` array. The entry should hold a display name for the family, an internal name, and a pointer to the afforementioned ``user_x_gen()`` function.
- Add a preprocessor ``#define`` to represent the index into said array, of the form ``GEN_MENU_x_ENTRY``.
