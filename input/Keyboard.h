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
