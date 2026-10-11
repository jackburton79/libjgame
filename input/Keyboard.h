/*
 * Keyboard.h
 */

#pragma once

#include "SupportDefs.h"

// Live keyboard state, and whether typed text is delivered
class Keyboard {
public:
	// The keys games can poll. Kept independent from SDL.
	enum Key {
		KEY_NONE = 0,
		KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
		KEY_W, KEY_A, KEY_S, KEY_D,
		KEY_E, KEY_P, KEY_R, KEY_Z, KEY_X,
		KEY_SPACE, KEY_RETURN, KEY_ESCAPE,
		KEY_LSHIFT, KEY_RSHIFT,
		// The rest of the letters, the digits (the top row), the function keys and the
		// editing and modifier keys
		KEY_B, KEY_C, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K,
		KEY_L, KEY_M, KEY_N, KEY_O, KEY_Q, KEY_T, KEY_U, KEY_V,
		KEY_Y,
		KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
		KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
		KEY_TAB, KEY_BACKSPACE, KEY_DELETE, KEY_INSERT, KEY_HOME, KEY_END,
		KEY_PAGEUP, KEY_PAGEDOWN, KEY_LCTRL, KEY_RCTRL, KEY_LALT, KEY_RALT,
		KEY_COUNT
	};

	static bool IsShiftPressed();

	// Whether the key is held down right now
	static bool IsKeyDown(Key key);
	// Whether the key went down since the last Events::Pump() (no auto-repeat)
	static bool WasKeyPressed(Key key);

	// Text input (the characters typed, as the layout makes them) is on while a
	// text field has the focus.
	static void StartTextInput();
	static void StopTextInput();

private:
	friend class Events;
	static void _BeginFrame();
	static void _KeyDown(int scancode);

	static bool sPressed[KEY_COUNT];
};
