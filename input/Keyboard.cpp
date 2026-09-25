/*
 * Keyboard.cpp
 */

#include "Keyboard.h"

#include <SDL.h>


/* static */
bool
Keyboard::IsShiftPressed()
{
	return (SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0;
}


/* static */
void
Keyboard::StartTextInput()
{
	SDL_StartTextInput();
}


/* static */
void
Keyboard::StopTextInput()
{
	SDL_StopTextInput();
}
