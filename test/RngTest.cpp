// The same seed must give the same sequence on every platform, and the
// helpers must stay inside their ranges.
#include "Rng.h"

#include <cstdio>
#include <cstdlib>

int
main()
{
	int failures = 0;

	// SplitMix64 reference values for seed 0 (Rng draws one value when seeded)
	Rng reference(0);
	const uint64_t expected[3] = {
		0x6E789E6AA1B965F4ull, 0x06C45D188009454Full, 0xF88BB8A8724C81ECull
	};
	for (int i = 0; i < 3; i++) {
		uint64_t value = reference.Next();
		if (value != expected[i]) {
			std::printf("FAIL: value %d is %016llx, expected %016llx\n", i,
				(unsigned long long)value, (unsigned long long)expected[i]);
			failures++;
		}
	}

	Rng a(1234), b(1234), c(1235);
	bool differs = false;
	for (int i = 0; i < 100; i++) {
		uint64_t value = a.Next();
		if (value != b.Next()) {
			std::printf("FAIL: the same seed gave different values\n");
			failures++;
			break;
		}
		differs = differs || value != c.Next();
	}
	if (!differs) {
		std::printf("FAIL: different seeds gave the same values\n");
		failures++;
	}

	Rng rng(42);
	bool sawLow = false, sawHigh = false;
	for (int i = 0; i < 10000; i++) {
		int n = rng.Int(-3, 4);
		float f = rng.Float();
		float r = rng.Range(2.0f, 5.0f);
		if (n < -3 || n > 4 || f < 0.0f || f >= 1.0f || r < 2.0f || r >= 5.0f) {
			std::printf("FAIL: out of range: Int %d, Float %f, Range %f\n", n, f, r);
			failures++;
			break;
		}
		sawLow = sawLow || n == -3;
		sawHigh = sawHigh || n == 4;
	}
	if (!sawLow || !sawHigh) {
		std::printf("FAIL: Int() never gave the ends of its range\n");
		failures++;
	}
	if (rng.Chance(0.0f) || !rng.Chance(1.0f)) {
		std::printf("FAIL: Chance(0) must be false and Chance(1) true\n");
		failures++;
	}

	if (failures == 0)
		std::printf("RngTest: all good\n");
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
