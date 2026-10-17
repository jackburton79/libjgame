/*
 * Keyboard.cpp
 */

#include <jgame/Keyboard.h>

#include <SDL.h>

namespace jgame {


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
		case Keyboard::KEY_B:		return SDL_SCANCODE_B;
		case Keyboard::KEY_C:		return SDL_SCANCODE_C;
		case Keyboard::KEY_F:		return SDL_SCANCODE_F;
		case Keyboard::KEY_G:		return SDL_SCANCODE_G;
		case Keyboard::KEY_H:		return SDL_SCANCODE_H;
		case Keyboard::KEY_I:		return SDL_SCANCODE_I;
		case Keyboard::KEY_J:		return SDL_SCANCODE_J;
		case Keyboard::KEY_K:		return SDL_SCANCODE_K;
		case Keyboard::KEY_L:		return SDL_SCANCODE_L;
		case Keyboard::KEY_M:		return SDL_SCANCODE_M;
		case Keyboard::KEY_N:		return SDL_SCANCODE_N;
		case Keyboard::KEY_O:		return SDL_SCANCODE_O;
		case Keyboard::KEY_Q:		return SDL_SCANCODE_Q;
		case Keyboard::KEY_T:		return SDL_SCANCODE_T;
		case Keyboard::KEY_U:		return SDL_SCANCODE_U;
		case Keyboard::KEY_V:		return SDL_SCANCODE_V;
		case Keyboard::KEY_Y:		return SDL_SCANCODE_Y;
		case Keyboard::KEY_0:		return SDL_SCANCODE_0;
		case Keyboard::KEY_1:		return SDL_SCANCODE_1;
		case Keyboard::KEY_2:		return SDL_SCANCODE_2;
		case Keyboard::KEY_3:		return SDL_SCANCODE_3;
		case Keyboard::KEY_4:		return SDL_SCANCODE_4;
		case Keyboard::KEY_5:		return SDL_SCANCODE_5;
		case Keyboard::KEY_6:		return SDL_SCANCODE_6;
		case Keyboard::KEY_7:		return SDL_SCANCODE_7;
		case Keyboard::KEY_8:		return SDL_SCANCODE_8;
		case Keyboard::KEY_9:		return SDL_SCANCODE_9;
		case Keyboard::KEY_F1:		return SDL_SCANCODE_F1;
		case Keyboard::KEY_F2:		return SDL_SCANCODE_F2;
		case Keyboard::KEY_F3:		return SDL_SCANCODE_F3;
		case Keyboard::KEY_F4:		return SDL_SCANCODE_F4;
		case Keyboard::KEY_F5:		return SDL_SCANCODE_F5;
		case Keyboard::KEY_F6:		return SDL_SCANCODE_F6;
		case Keyboard::KEY_F7:		return SDL_SCANCODE_F7;
		case Keyboard::KEY_F8:		return SDL_SCANCODE_F8;
		case Keyboard::KEY_F9:		return SDL_SCANCODE_F9;
		case Keyboard::KEY_F10:		return SDL_SCANCODE_F10;
		case Keyboard::KEY_F11:		return SDL_SCANCODE_F11;
		case Keyboard::KEY_F12:		return SDL_SCANCODE_F12;
		case Keyboard::KEY_TAB:	return SDL_SCANCODE_TAB;
		case Keyboard::KEY_BACKSPACE:	return SDL_SCANCODE_BACKSPACE;
		case Keyboard::KEY_DELETE:	return SDL_SCANCODE_DELETE;
		case Keyboard::KEY_INSERT:	return SDL_SCANCODE_INSERT;
		case Keyboard::KEY_HOME:	return SDL_SCANCODE_HOME;
		case Keyboard::KEY_END:	return SDL_SCANCODE_END;
		case Keyboard::KEY_PAGEUP:	return SDL_SCANCODE_PAGEUP;
		case Keyboard::KEY_PAGEDOWN:	return SDL_SCANCODE_PAGEDOWN;
		case Keyboard::KEY_LCTRL:	return SDL_SCANCODE_LCTRL;
		case Keyboard::KEY_RCTRL:	return SDL_SCANCODE_RCTRL;
		case Keyboard::KEY_LALT:	return SDL_SCANCODE_LALT;
		case Keyboard::KEY_RALT:	return SDL_SCANCODE_RALT;
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

} // namespace jgame
