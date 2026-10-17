/*
 * ZLibDecompressor.h
 *
 *  Created on: 19/nov/2014
 *      Author: Stefano Ceccherini
 */

#pragma once

#include <jgame/SupportDefs.h>

namespace jgame {


class ZLibDecompressor {
public:
	ZLibDecompressor();
	~ZLibDecompressor();

	static status_t DecompressBuffer(const void* inputBuffer,
								const size_t& inputSize,
								void* outputBuffer,
								size_t& outputSize);
};

} // namespace jgame
