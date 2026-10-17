/*
 * Gamepad.h
 *
 * Game controllers (the ones SDL knows as "game controllers": the usual pads
 * of consoles and PCs) as keys. Events::Pump() opens the pads as they are
 * plugged in and keeps their state, and Keyboard::IsKeyDown() and
 * Keyboard::WasKeyPressed() answer for them too, as if the keys they stand
 * for were pressed: a game that reads the keyboard works with a pad without
 * changing a line.
 *
 *   left stick, D-pad     Up, Down, Left, Right
 *   A                     Space and Return
 *   B, right trigger      X
 *   X                     E
 *   Y                     R
 *   Start                 P
 *   Back                  Escape
 *
 * A stick or a trigger counts as pressed past half of its travel.
 */
#pragma once

union SDL_Event;

namespace jgame {

class Gamepad {
public:
	// Whether at least one pad is plugged in
	static bool IsConnected();

private:
	friend class Events;
	friend class Keyboard;
	static void _Initialize();
	static void _HandleEvent(const union SDL_Event& event);
	static void _Handle(const union SDL_Event& event);
	static bool _IsKeyDown(int key);
};

} // namespace jgame
