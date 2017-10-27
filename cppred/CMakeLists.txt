cmake_minimum_required (VERSION 3.0)

project (cppred)

file(GLOB SOURCES "*.cpp")

INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})

option(BUILD_RED_VERSION "Build Red version" ON)

if(BUILD_RED_VERSION)
  add_definitions(-DPOKEMON_VERSION=RED)
else()
  add_definitions(-DPOKEMON_VERSION=BLUE)
endif()

add_executable(cppred ${SOURCES})
target_link_libraries(cppred ${SDL2_LIBRARIES} pthread boost_coroutine boost_context)
