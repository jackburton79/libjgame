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
