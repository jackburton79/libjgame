/*
 * Timer.h
 *
 *  Created on: 11/ago/2012
 *      Author: stefano
 */

#pragma once

#include "SupportDefs.h"

#include <map>
#include <string>


typedef void (*timer_function)(void* parameter);


class Timer {
public:
	static bool Initialize();
	static void TearDown();
	static Timer* Set(const char* name, uint32 delay);
	static Timer* Get(const char* name);

	bool Expired();
	void Rearm();

	static void Wait(uint32 delay);
	static void WaitSync(uint32 start, uint32 maxDelay);
	static int AddOneShotTimer(uint32 time, timer_function func, void* parameter);
	static int AddPeriodicTimer(uint32 interval, timer_function func, void* parameter);
	static void RemovePeriodicTimer(int id);

	static uint32 Ticks();

	class Functor {
	public:
		Functor(timer_function func, void* parameter);
		timer_function& Function();
		void* Parameter();

	private:
		timer_function fFunction;
		void* fParameter;
	};

private:
	Timer(uint32 delay);

	uint32 fDelay;
	uint32 fExpirationTime;

	typedef std::map<std::string, Timer*> timer_map;
	static timer_map sNamedTimers;
};

