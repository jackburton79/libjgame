/*
 * Events.h
 */

#pragma once

// The event pump. Call Pump() once per frame: it refreshes the state
// Keyboard::WasKeyPressed() reports, runs the callbacks of the timers
// (see Timer::AddOneShotTimer() and AddPeriodicTimer()), and notes
// when the user asks to close the window.
class Events {
public:
	static void Pump();

	static bool QuitRequested();
	static void RequestQuit();
};
