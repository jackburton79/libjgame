#include "Events.h"
#include "Keyboard.h"

#include <SDL.h>

#include <cstdio>
#include <cstdlib>

static int sFailures = 0;
static int sTimerCalls = 0;

#define CHECK(cond, msg) \
	do { \
		if (!(cond)) { \
			std::printf("FAIL: %s\n", msg); \
			sFailures++; \
		} \
	} while (0)

static void
OnTimer(void* parameter)
{
	sTimerCalls += *static_cast<int*>(parameter);
}

static void
PushKey(SDL_Scancode scancode, Uint8 repeat)
{
	SDL_Event event;
	SDL_zero(event);
	event.type = SDL_KEYDOWN;
	event.key.keysym.scancode = scancode;
	event.key.repeat = repeat;
	SDL_PushEvent(&event);
}

int
main()
{
	if (SDL_Init(SDL_INIT_EVENTS) != 0) {
		std::printf("SDL_Init: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	CHECK(!Keyboard::WasKeyPressed(Keyboard::KEY_SPACE), "nothing pressed yet");

	PushKey(SDL_SCANCODE_SPACE, 0);
	PushKey(SDL_SCANCODE_W, 1);		// auto-repeat: ignored
	Events::Pump();
	CHECK(Keyboard::WasKeyPressed(Keyboard::KEY_SPACE), "space went down");
	CHECK(!Keyboard::WasKeyPressed(Keyboard::KEY_W), "auto-repeat is not a press");

	// The edge lasts one pump only
	Events::Pump();
	CHECK(!Keyboard::WasKeyPressed(Keyboard::KEY_SPACE), "press is reported once");

	// Timer callbacks run from Pump()
	int amount = 3;
	SDL_Event user;
	SDL_zero(user);
	user.type = SDL_USEREVENT;
	user.user.data1 = reinterpret_cast<void*>(&OnTimer);
	user.user.data2 = &amount;
	SDL_PushEvent(&user);
	Events::Pump();
	CHECK(sTimerCalls == 3, "timer callback ran");

	CHECK(!Events::QuitRequested(), "no quit yet");
	SDL_Event quit;
	SDL_zero(quit);
	quit.type = SDL_QUIT;
	SDL_PushEvent(&quit);
	Events::Pump();
	CHECK(Events::QuitRequested(), "quit requested");

	SDL_Quit();
	if (sFailures == 0)
		std::printf("EventsTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
