/*
 * Keyboard.h
 */

#pragma once

// Live keyboard modifier state, and whether typed text is delivered
class Keyboard {
public:
	static bool IsShiftPressed();

	// Text input (the characters typed, as the layout makes them) is on while a
	// text field has the focus.
	static void StartTextInput();
	static void StopTextInput();
};
