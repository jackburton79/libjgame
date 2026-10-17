/*
 * StringStream.h
 *
 *  Created on: 19/mag/2012
 *      Author: Stefano Ceccherini
 */

#pragma once

#include <jgame/MemoryStream.h>

#include <string>

namespace jgame {

class StringStream: public MemoryStream {
public:
	StringStream(const char* string);
	StringStream(const std::string& string);
	virtual ~StringStream();
};

} // namespace jgame
