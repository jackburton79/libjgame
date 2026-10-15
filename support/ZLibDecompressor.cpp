/*
 * ZLibDecompressor.cpp
 *
 *  Created on: 19/nov/2014
 *      Author: Stefano Ceccherini
 */

#include "ZLibDecompressor.h"

#include <iostream>

#include <zlib.h>


ZLibDecompressor::ZLibDecompressor()
{
}


ZLibDecompressor::~ZLibDecompressor()
{
}


/* static */
status_t
ZLibDecompressor::DecompressBuffer(const void* inputBuffer,
		const size_t& inputSize, void* outputBuffer,
		size_t& outputSize)
{
	// zlib's length is an unsigned long, which is not a size_t everywhere (on 64 bit Windows it is half as
	// wide): go through a variable of its own type, not a cast of the pointer
	uLongf destinationSize = static_cast<uLongf>(outputSize);
	int status = uncompress(static_cast<Bytef*>(outputBuffer), &destinationSize,
							static_cast<const Bytef*>(inputBuffer), static_cast<uLong>(inputSize));
	outputSize = static_cast<size_t>(destinationSize);

	if (status != Z_OK) {
		std::cerr << "ZLibDecompressor::DecompressBuffer(): ";
		std::cerr << zError(status);
		return -1;
	}

	return 0;
}
