/*
 * Events.cpp
 */

#include <jgame/Events.h>

#include <jgame/Gamepad.h>
#include <jgame/Keyboard.h>
#include <jgame/Timer.h>

#include <SDL.h>

namespace jgame {


static bool sQuitRequested = false;


/* static */
void
Events::Pump()
{
	Gamepad::_Initialize();
	Keyboard::_BeginFrame();

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				sQuitRequested = true;
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

} // namespace jgame
