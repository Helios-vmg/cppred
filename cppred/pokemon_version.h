#pragma once

#define RED 0x3ED
#define BLUE 0xB100

#if !defined POKEMON_VERSION || (POKEMON_VERSION != RED && POKEMON_VERSION != BLUE)
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
