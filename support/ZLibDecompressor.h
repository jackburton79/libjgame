/*
 * ZLibDecompressor.h
 *
 *  Created on: 19/nov/2014
 *      Author: Stefano Ceccherini
 */

#pragma once

#include "SupportDefs.h"


class ZLibDecompressor {
public:
	ZLibDecompressor();
	~ZLibDecompressor();

	static status_t DecompressBuffer(const void* inputBuffer,
								const size_t& inputSize,
								void* outputBuffer,
								size_t& outputSize);
};
