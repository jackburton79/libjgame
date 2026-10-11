/*
 * Rng.h
 *
 * Small deterministic RNG (SplitMix64). std::mt19937 is portable but the
 * <random> distributions are not, and a seed must give the same sequence
 * everywhere. Header only: it needs nothing else from the library.
 */
#pragma once

#include <cstdint>

class Rng {
public:
	explicit Rng(uint64_t seed = 1) : fState(seed) { Next(); }

	uint64_t Next()
	{
		fState += 0x9E3779B97F4A7C15ull;
		uint64_t z = fState;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	// Inclusive range
	int Int(int low, int high)
	{
		return low + (int)(Next() % (uint64_t)(high - low + 1));
	}

	// [0, 1)
	float Float() { return (float)(Next() >> 40) * (1.0f / 16777216.0f); }
	float Range(float low, float high) { return low + Float() * (high - low); }
	bool Chance(float probability) { return Float() < probability; }

private:
	uint64_t fState;
};
