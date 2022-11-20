# Cycle Games on Graphs

Computer code for playing the combinatorial games <i>Make-A-Cycle</i> (MAC) and <i>Avoid-A-Cycle</i> (AAC). These games were first defined for Cayley graphs in the paper <a href="http://dx.doi.org/10.1515/9783110755411-011">Relator Games on Groups</a>. A second paper discussing the games MAC and AAC is forthcoming (put pre-print link here when ready).

## Compiling/ Building

Download the folder <b>Cycle Games on Graphs</b>. The project can be built by compiling Source.cpp using at least C++17.  

The project can be built with the provided <b>Makefile<b>, or simply with ```g++ --std=c++20 Source.cpp```.
  

## Notes

Due to the difference in newline characters between Windows, Mac, and Linux, the provided adjacency files may not work on your machine. This will become evident when you attempt to use a file to simulate a game and received an error while parsing it. There's nothing sacred about the provided adjaency info files in the repo. They can always be regenerated using the tool.

This project originally just required c++17 (due to its usage of std::filesystem) but now requires c++20 due to its usage of the __VA_OPT__ functional macro. I believe this means g++ version 10.0 or newer is now needed to compile the project. While I don't anticipate this being an issue, if it proves to be I can make some compatability changes in the code that will allow it to be built (albeit with less informative error reporting at runtime). 
