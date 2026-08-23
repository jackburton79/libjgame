/*
 * Timer.cpp
 *
 *  Created on: 11/ago/2012
 *      Author: stefano
 */


#include "Timer.h"


#include <iostream>
#include <sys/time.h>

#include <SDL.h>


Timer::timer_map Timer::sNamedTimers;

static std::map<SDL_TimerID, Timer::Functor*> sPeriodicTimers;

Timer::Timer(uint32 delay)
	:
	fDelay(delay),
	fExpirationTime(Ticks() + delay)
{
}


/* static */
bool
Timer::Initialize()
{
	if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0)
		return false;
	return true;
}


/* static */
void
Timer::TearDown()
{
	for (auto timer : sNamedTimers) {
		delete timer.second;
	}
	sNamedTimers.clear();

	for (auto [id, functor] : sPeriodicTimers) {
		delete functor;
	}
	sPeriodicTimers.clear();

}


/* static */
Timer*
Timer::Set(const char* name, uint32 delay)
{
	sNamedTimers[name] = new Timer(delay);
	return sNamedTimers[name];
}


/*static */
Timer*
Timer::Get(const char* name)
{
	auto i = sNamedTimers.find(name);
	if (i == sNamedTimers.end())
		return NULL;
	return i->second;
}


bool
Timer::Expired()
{
	return Ticks() >= fExpirationTime;
}


void
Timer::Rearm()
{
	fExpirationTime = Ticks() + fDelay;
}


static uint32
oneshot_timer_callback(uint32 interval, void* castToFunctor)
{
	Timer::Functor* functor = reinterpret_cast<Timer::Functor*>(castToFunctor);
	SDL_Event event;
	SDL_UserEvent userevent;

	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = (void*)functor->Function();
	userevent.data2 = functor->Parameter();

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);

	delete functor;

	return 0;
}


static uint32
periodic_timer_callback(uint32 interval, void* castToFunctor)
{
	Timer::Functor* functor = reinterpret_cast<Timer::Functor*>(castToFunctor);

	SDL_Event event;
	SDL_UserEvent userevent;

	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = (void*)functor->Function();
	userevent.data2 = functor->Parameter();

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);

	return interval;
}


/* static */
int
Timer::AddOneShotTimer(uint32 delay, timer_function func, void* parameter)
{
	Functor* functor = new Functor(func, parameter);
	SDL_TimerID id = SDL_AddTimer(delay, oneshot_timer_callback, (void*)functor);
	return id;
}


/* static */
int
Timer::AddPeriodicTimer(uint32 interval, timer_function func, void* parameter)
{
	Functor* functor = new Functor(func, parameter);
	SDL_TimerID id = SDL_AddTimer(interval, periodic_timer_callback, (void*)functor);
	sPeriodicTimers[id] = functor;
	return id;
}


void
Timer::RemovePeriodicTimer(int id)
{
	SDL_RemoveTimer(id);
	auto t = sPeriodicTimers.find(id);
	if (t != sPeriodicTimers.end())
		delete t->second;
	sPeriodicTimers.erase(t);
}


/* static */
uint32
Timer::Ticks()
{
	return SDL_GetTicks();
}


/* static */
void
Timer::Wait(uint32 delay)
{
	SDL_Delay(delay);
}


/* static */
void
Timer::WaitSync(uint32 start, uint32 maxDelay)
{
	int32 diff = (start + maxDelay) - Ticks();
	if (diff > 0)
		Wait(diff);
#if 0
	else
		std::cerr << Log::Yellow << "WaitSync: TOO SLOW!" << std::endl;
	std::cerr << Log::Normal;
#endif
}


// Functor
Timer::Functor::Functor(timer_function func, void* parameter)
	:
	fFunction(func),
	fParameter(parameter)
{
}


timer_function&
Timer::Functor::Function()
{
	return fFunction;
}


void*
Timer::Functor::Parameter()
{
	return fParameter;
}
