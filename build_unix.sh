#!/bin/sh

cpu_count=$(nproc)

# Build FreeImage
cd FreeImage
make -j $cpu_count
cd ..

# Build code generator
cd code_generation
cmake .
make -j $cpu_count
cd ..

# Generate static resources
cd CodeGeneration
../code_generation/code_generation
cd ..

# Build game
cd cppred
cmake .
make -j $cpu_count
cd ..
