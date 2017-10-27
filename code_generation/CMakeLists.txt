cmake_minimum_required (VERSION 3.0)

project (code_generation)

file(GLOB SOURCES "*.cpp")

add_executable(code_generation ${SOURCES} ../common/sha1.cpp)
target_link_libraries (code_generation freeimage)
