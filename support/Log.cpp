/*
 * Log.cpp
 *
 *  Created on: 13 dic 2020
 *      Author: Stefano Ceccherini
 */

#include <jgame/Log.h>

namespace jgame {

const char* Log::Normal = "\033[0m";
const char* Log::Green = "\033[1;32m";
const char* Log::Red = "\033[1;31m";
const char* Log::Yellow = "\033[1;33m";


/* static */
std::string
Log::InGreen(const std::string& text)
{
	return std::string(Green).append(text).append(Normal);
}


/* static */
std::string
Log::InRed(const std::string& text)
{
	return std::string(Red).append(text).append(Normal);
}

} // namespace jgame
