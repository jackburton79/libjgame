/*
 * Events.cpp
 */

#include <jgame/Events.h>

#include <jgame/Gamepad.h>
#include <jgame/Keyboard.h>
#include <jgame/Timer.h>
#include <jgame/TouchControls.h>

#include <SDL.h>

namespace jgame {


static bool sQuitRequested = false;
static bool sInBackground = false;
static bool sWentToBackground = false;
static bool sCameToForeground = false;


/* static */
void
Events::Pump()
{
	Gamepad::_Initialize();
	Keyboard::_BeginFrame();
	TouchControls::_BeginFrame();
	sWentToBackground = sCameToForeground = false;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		TouchControls::_HandleEvent(event);
		switch (event.type) {
			case SDL_QUIT:
			case SDL_APP_TERMINATING:
				sQuitRequested = true;
				break;
			case SDL_APP_WILLENTERBACKGROUND:
			case SDL_APP_DIDENTERBACKGROUND:
				if (!sInBackground)
					sWentToBackground = true;
				sInBackground = true;
				break;
			case SDL_APP_WILLENTERFOREGROUND:
			case SDL_APP_DIDENTERFOREGROUND:
				if (sInBackground)
					sCameToForeground = true;
				sInBackground = false;
				break;
			case SDL_KEYDOWN:
				if (!event.key.repeat)
					Keyboard::_KeyDown(event.key.keysym.scancode);
				break;
			case SDL_USEREVENT:
				// Posted by the timers: data1 is the function, data2 its parameter
				if (event.user.data1 != nullptr) {
					timer_function function
						= reinterpret_cast<timer_function>(event.user.data1);
					function(event.user.data2);
				}
				break;
			default:
				Gamepad::_HandleEvent(event);
				break;
		}
	}
}


/* static */
bool
Events::QuitRequested()
{
	return sQuitRequested;
}


/* static */
void
Events::RequestQuit()
{
	sQuitRequested = true;
}


/* static */
bool
Events::IsInBackground()
{
	return sInBackground;
}


/* static */
bool
Events::WentToBackground()
{
	return sWentToBackground;
}


/* static */
bool
Events::CameToForeground()
{
	return sCameToForeground;
}

} // namespace jgame
