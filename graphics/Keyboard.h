/*
 * Keyboard.h
 */

#pragma once

// Live keyboard modifier state, for a caller that needs to distinguish a
// plain click/action from a shift-modified one (e.g. "replace selection"
// vs "add to selection") without depending on the underlying windowing/
// input library directly.
class Keyboard {
public:
	static bool IsShiftPressed();
};
