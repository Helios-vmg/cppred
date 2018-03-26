cmake_minimum_required (VERSION 3.0)

project (cppred)

file(GLOB SOURCES "*.cpp")
file(GLOB CPPRED_SOURCES "CppRed/*.cpp")
file(GLOB SCRIPTS_SOURCES "CppRed/Scripts/*.cpp")

INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS} .)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(cppred ${SOURCES} ${CPPRED_SOURCES} ${SCRIPTS_SOURCES})
target_link_libraries(cppred ${SDL2_LIBRARIES} pthread boost_coroutine boost_context)
