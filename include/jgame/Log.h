/*
 * Log.h
 *
 *  Created on: 12 dic 2020
 *      Author: Stefano Ceccherini
 */

#pragma once

#include <string>

namespace jgame {

class Log {
public:
	static const char* Green;
	static const char* Normal;
	static const char* Red;
	static const char* Yellow;

	// The text, in that colour (ANSI escape codes)
	static std::string InGreen(const std::string& text);
	static std::string InRed(const std::string& text);
};

} // namespace jgame
