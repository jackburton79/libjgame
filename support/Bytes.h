/*
 * Bytes.h
 *
 * Little-endian numbers in a byte buffer, whatever the machine, and a checksum
 * (FNV-1a): what a save file format needs. Header only: it needs nothing else
 * from the library.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Bytes {

inline uint32_t
Checksum(const uint8_t* data, size_t size)
{
	uint32_t h = 2166136261u;
	for (size_t i = 0; i < size; i++)
		h = (h ^ data[i]) * 16777619u;
	return h;
}


inline void
Put(std::vector<uint8_t>& out, uint64_t value, int bytes)
{
	for (int i = 0; i < bytes; i++)
		out.push_back((uint8_t)(value >> (8 * i)));
}


inline uint64_t
Get(const uint8_t* data, int bytes)
{
	uint64_t value = 0;
	for (int i = 0; i < bytes; i++)
		value |= (uint64_t)data[i] << (8 * i);
	return value;
}

} // namespace Bytes
