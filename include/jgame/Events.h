/*
 * Events.h
 */

#pragma once

namespace jgame {

// The event pump. Call Pump() once per frame: it refreshes the state
// Keyboard::WasKeyPressed() reports (keys, game controllers and touch
// controls), runs the callbacks of the timers (see Timer::AddOneShotTimer()
// and AddPeriodicTimer()), and notes when the user asks to close the window.
class Events {
public:
	static void Pump();

	static bool QuitRequested();
	static void RequestQuit();

	// Phones and tablets (Android, iOS): the system puts the program in the
	// background whenever the user goes to another application, to the home
	// screen or answers a call, and may end it there without a word. A game
	// should then pause, and save what it wants to keep: WentToBackground() is
	// true for the Pump() that saw it happen (it may be the last). While in the
	// background (IsInBackground()) nothing is shown, and on Android SDL waits
	// in Pump() until the program comes back; the sound is paused by SDL
	// meanwhile. CameToForeground() is true for the Pump() that saw it come
	// back. On other systems none of this happens: the three are always false.
	static bool IsInBackground();
	static bool WentToBackground();
	static bool CameToForeground();
};

} // namespace jgame
