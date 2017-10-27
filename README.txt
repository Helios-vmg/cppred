CppRed - A port of the original Pokemon Red/Blue for Gameboy to modern C++. 


                                 PROJECT GOALS

* Produce code that's easy to study and modify.
* 100% accurate behavior is not a goal, but in some cases some effort will be
  used to preserve specific behaviors, such as the famous Mew glitch.
* Possibility to produce different builds with various features, such as
  optional bug fixes or more faithful replication of original behavior.
* Maintain bidirectional compatibility with common emulator save files.


                                    BUILDING

Requirements:
* Latest Boost (specifically Boost.coroutine and Boost.context)
* SDL2


Windows

A Visual Studio solution is provided. SDL's and Boost's headers are expected to
be somewhere where VS can find them, on in $(SolutionDir)\include. Same for the
.libs, but in $(SolutionDir)\lib64.
To build just build the entire solution, and this takes care of everything. The
output goes to $(SolutionDir)\bin64.


Linux/BSD/etc.

Additional requirements:
* cmake

Run build_unix.sh. This should build everything. The output goes to ./bin.
