/*
 * Keyboard.cpp
 */

#include "Keyboard.h"

#include <SDL.h>


bool Keyboard::sPressed[Keyboard::KEY_COUNT];


static SDL_Scancode
scancode_for(Keyboard::Key key)
{
	switch (key) {
		case Keyboard::KEY_UP:		return SDL_SCANCODE_UP;
		case Keyboard::KEY_DOWN:	return SDL_SCANCODE_DOWN;
		case Keyboard::KEY_LEFT:	return SDL_SCANCODE_LEFT;
		case Keyboard::KEY_RIGHT:	return SDL_SCANCODE_RIGHT;
		case Keyboard::KEY_W:		return SDL_SCANCODE_W;
		case Keyboard::KEY_A:		return SDL_SCANCODE_A;
		case Keyboard::KEY_S:		return SDL_SCANCODE_S;
		case Keyboard::KEY_D:		return SDL_SCANCODE_D;
		case Keyboard::KEY_E:		return SDL_SCANCODE_E;
		case Keyboard::KEY_P:		return SDL_SCANCODE_P;
		case Keyboard::KEY_R:		return SDL_SCANCODE_R;
		case Keyboard::KEY_Z:		return SDL_SCANCODE_Z;
		case Keyboard::KEY_X:		return SDL_SCANCODE_X;
		case Keyboard::KEY_SPACE:	return SDL_SCANCODE_SPACE;
		case Keyboard::KEY_RETURN:	return SDL_SCANCODE_RETURN;
		case Keyboard::KEY_ESCAPE:	return SDL_SCANCODE_ESCAPE;
		case Keyboard::KEY_LSHIFT:	return SDL_SCANCODE_LSHIFT;
		case Keyboard::KEY_RSHIFT:	return SDL_SCANCODE_RSHIFT;
		default:					return SDL_SCANCODE_UNKNOWN;
	}
}


/* static */
bool
Keyboard::IsShiftPressed()
{
	return (SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0;
}


/* static */
bool
Keyboard::IsKeyDown(Key key)
{
	const SDL_Scancode scancode = scancode_for(key);
	if (scancode == SDL_SCANCODE_UNKNOWN)
		return false;
	return SDL_GetKeyboardState(nullptr)[scancode] != 0;
}


/* static */
bool
Keyboard::WasKeyPressed(Key key)
{
	return key > KEY_NONE && key < KEY_COUNT && sPressed[key];
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


/* static */
void
Keyboard::_BeginFrame()
{
	for (int i = 0; i < KEY_COUNT; i++)
		sPressed[i] = false;
}


/* static */
void
Keyboard::_KeyDown(int scancode)
{
	for (int i = KEY_NONE + 1; i < KEY_COUNT; i++) {
		if (scancode_for((Key)i) == scancode)
			sPressed[i] = true;
	}
}
