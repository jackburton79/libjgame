// Mirror() used to swap bytes instead of pixels, which only works on 8 bit surfaces.
#include "Bitmap.h"

#include <cstdio>
#include <cstdlib>

static int
TestDepth(int depth)
{
	// An odd width, to have a middle column that stays where it is
	const int kWidth = 5, kHeight = 3;
	Bitmap* bitmap = new Bitmap(kWidth, kHeight, depth);
	uint32 colors[kHeight][kWidth];
	for (int y = 0; y < kHeight; y++) {
		for (int x = 0; x < kWidth; x++) {
			colors[y][x] = depth == 8 ? (uint32)(y * kWidth + x + 1)
				: bitmap->MapRGBColor(40 * x + 10, 70 * y + 20, 200 - 30 * x);
			bitmap->PutPixel(x, y, colors[y][x]);
		}
	}

	bitmap->Mirror();

	int failures = 0;
	for (int y = 0; y < kHeight; y++) {
		for (int x = 0; x < kWidth; x++) {
			const uint32 expected = colors[y][kWidth - 1 - x];
			if (bitmap->GetPixel(x, y) != expected) {
				std::printf("FAIL: %d bit, pixel %d,%d is %08x, expected %08x\n",
					depth, x, y, bitmap->GetPixel(x, y), expected);
				failures++;
			}
		}
	}
	bitmap->Release();
	return failures;
}

int
main()
{
	int failures = 0;
	const int depths[] = { 8, 16, 24, 32 };
	for (int depth : depths)
		failures += TestDepth(depth);

	if (failures == 0)
		std::printf("MirrorTest: all good\n");
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
