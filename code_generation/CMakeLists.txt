cmake_minimum_required (VERSION 3.0)

project (code_generation)

file(GLOB SOURCES "*.cpp")

link_directories(../FreeImage/Dist)
add_executable(code_generation ${SOURCES} ../common/sha1.cpp)
include_directories(AFTER SYSTEM ../FreeImage/Dist)
target_link_libraries(code_generation freeimage)
