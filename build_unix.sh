#!/bin/sh

cpu_count=$(nproc)

cd code_generation
cmake .
make -j $cpu_count
cd ../CodeGeneration
../code_generation/code_generation
cd ../cppred
cmake .
make -j $cpu_count
