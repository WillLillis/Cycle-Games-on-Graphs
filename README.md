# Cycle Games on Graphs

Computer code for playing the combinatorial games <i>Make-A-Cycle</i> (MAC) and <i>Avoid-A-Cycle</i> (AAC). These games were first defined for Cayley graphs in the paper <a href="http://dx.doi.org/10.1515/9783110755411-011">Relator Games on Groups</a>. A second paper discussing the games MAC and AAC is forthcoming (put pre-print link here when ready).

## Compiling/ Building

Download the folder <b>Cycle Games on Graphs</b>. The project can be built by compiling Source.cpp using at least <tt>C++17<tt>.  

The project can be built with the provided <tt>Makefile<tt>, or simply with <tt>g++ --std=c++17 Source.cpp<tt>

## Notes

Due to the difference in newline characters between Windows, Mac, and Linux, the provided adjacency files may not work on your machine. This will become evident when you attempt to use a file to simulate a game and received an error while parsing it. There's nothing sacred about the provided adjaency info files in the repo. They can always be regenerated using the tool.
