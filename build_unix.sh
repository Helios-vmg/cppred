#!/bin/sh

cpu_count=$(nproc)

cd code_generator
./generate_makefile.py
make -j $cpu_count
cd ../generated_files
../code_generator/code_generator cpu.generated.h cpu.generated.cpp
cd ../pdboy

LIBS=$(pkg-config --libs sdl2)
INCLUDES=$(pkg-config --cflags-only-I sdl2)
export LIBS
export INCLUDES

./generate_makefile.py
make -j $cpu_count
