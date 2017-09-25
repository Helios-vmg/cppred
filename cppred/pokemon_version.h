#pragma once

#define RED 'red'
#define BLUE 'blue'

#if POKEMON_VERSION != RED && POKEMON_VERSION != BLUE
#error Pokemon version not defined!
#endif

/*
To check:

#if POKEMON_VERSION == RED
//...
#elif POKEMON_VERSION == BLUE
//...
#endif

*/
