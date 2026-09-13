/*
 * Log.h
 *
 *  Created on: 12 dic 2020
 *      Author: Stefano Ceccherini
 */

#pragma once

#define GREEN(x) std::string("\033[1;32m").append(x).append("\033[0m")
#define RED(x) std::string("\033[1;31m").append(x).append("\033[0m")

class Log {
public:
	static const char* Green;
	static const char* Normal;
	static const char* Red;
	static const char* Yellow;
};
