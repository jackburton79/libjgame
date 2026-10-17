// Timer's one-shot and periodic callbacks: every one-shot runs once, even with
// no delay at all, removing a timer that isn't there is harmless, and nothing
// fires after TearDown().
#include "Events.h"
#include "Timer.h"

#include <SDL.h>

#include <cstdio>
#include <cstdlib>

static int sFailures = 0;
static int sOneShotCalls = 0;
static int sPeriodicCalls = 0;

#define CHECK(cond, msg) \
	do { \
		if (!(cond)) { \
			std::printf("FAIL: %s\n", msg); \
			sFailures++; \
		} \
	} while (0)

static void
OnOneShot(void*)
{
	sOneShotCalls++;
}

static void
OnPeriodic(void*)
{
	sPeriodicCalls++;
}

// Pumps until `counter` reaches `wanted`, for at most `timeout` ms
static void
PumpUntil(const int& counter, int wanted, uint32 timeout)
{
	const uint32 start = Timer::Ticks();
	while (counter < wanted && Timer::Ticks() - start < timeout) {
		Events::Pump();
		Timer::Wait(1);
	}
}

int
main()
{
	if (SDL_Init(SDL_INIT_EVENTS) != 0 || !Timer::Initialize()) {
		std::printf("SDL_Init: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	// No delay: they can fire before AddOneShotTimer() has returned
	const int kOneShots = 200;
	for (int i = 0; i < kOneShots; i++)
		CHECK(Timer::AddOneShotTimer(i % 2, OnOneShot, nullptr) != 0, "no timer id");
	PumpUntil(sOneShotCalls, kOneShots, 2000);
	Events::Pump();
	CHECK(sOneShotCalls == kOneShots, "a one-shot timer didn't run, or ran twice");

	// Nothing to remove: must not touch anything
	Timer::RemovePeriodicTimer(123456);

	const int id = Timer::AddPeriodicTimer(5, OnPeriodic, nullptr);
	PumpUntil(sPeriodicCalls, 3, 2000);
	CHECK(sPeriodicCalls >= 3, "the periodic timer didn't run");
	Timer::RemovePeriodicTimer(id);
	Timer::RemovePeriodicTimer(id);

	// Torn down with timers still pending: none may fire afterwards
	Timer::AddPeriodicTimer(5, OnPeriodic, nullptr);
	for (int i = 0; i < 50; i++)
		Timer::AddOneShotTimer(20, OnOneShot, nullptr);
	Timer::TearDown();
	Events::Pump();		// what fired before TearDown()
	const int oneShotsBefore = sOneShotCalls;
	const int periodicBefore = sPeriodicCalls;
	Timer::Wait(60);
	Events::Pump();
	CHECK(sOneShotCalls == oneShotsBefore, "a one-shot timer fired after TearDown()");
	CHECK(sPeriodicCalls == periodicBefore, "a periodic timer fired after TearDown()");

	SDL_Quit();

	if (sFailures == 0)
		std::printf("TimerTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
