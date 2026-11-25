/*
 * Platform.h
 *
 * What a game may want to know about the system it runs on, and where it can
 * keep its files. On a phone or a tablet (Android, iOS) there is no home
 * folder and no working directory a program can write in: the system gives
 * each application a folder of its own, and UserDataDirectory() is the way to
 * find it on every system.
 */
#pragma once

#include <string>

namespace jgame {

class Platform {
public:
	// Android or iOS: the window is the whole screen, there may be no keyboard
	// and the system can stop the program at any moment (see
	// Events::WentToBackground())
	static bool IsMobile();
	// Whether a touch screen is there (SDL's video must have been started,
	// by GraphicsEngine::Initialize(), for it to be seen)
	static bool HasTouchScreen();

	// The folder where the program can keep the user's files (profile,
	// options, saved games), created if it isn't there yet, with a '/' at the
	// end; empty if there is none. Linux: ~/.local/share/<application>, Mac:
	// ~/Library/Application Support/<application>, Windows:
	// %APPDATA%\<organization>\<application>, Android: the application's own
	// internal storage (the organization and the application are not used
	// there), iOS: the application's Library folder. `organization` may be
	// empty.
	static std::string UserDataDirectory(const char* organization, const char* application);
};

} // namespace jgame
