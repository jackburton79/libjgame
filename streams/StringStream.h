/*
 * StringStream.h
 *
 *  Created on: 19/mag/2012
 *      Author: Stefano Ceccherini
 */

#pragma once

#include "MemoryStream.h"

#include <string>

class StringStream: public MemoryStream {
public:
	StringStream(const char* string);
	StringStream(const std::string& string);
	virtual ~StringStream();
};
