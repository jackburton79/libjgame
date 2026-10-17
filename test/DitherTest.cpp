// FillRectDithered() paints the share of the pixels it is asked for, in a
// pattern that lines up from one rect to the next, and never outside the
// rect or the bitmap.
#include <jgame/Bitmap.h>

#include <cstdio>
#include <cstdlib>

using namespace jgame;

static int sFailures = 0;

#define CHECK(cond, msg) \
	do { \
		if (!(cond)) { \
			std::printf("FAIL: %s\\n", msg); \
			sFailures++; \
		} \
	} while (0)

static int
Count(Bitmap* bitmap, uint32 color)
{
	int count = 0;
	for (int y = 0; y < bitmap->Height(); y++)
		for (int x = 0; x < bitmap->Width(); x++)
			if (bitmap->GetPixel(x, y) == color)
				count++;
	return count;
}


int
main()
{
	Bitmap* bitmap = new Bitmap(16, 12, 32);
	const uint32 back = bitmap->MapRGBColor(0, 0, 255);
	const uint32 ink = bitmap->MapRGBColor(255, 255, 0);
	const GFX::rect all(0, 0, 16, 12);

	// As many of every 16 pixels as the level says, and each level keeps the
	// pixels of the one before
	bool grows = true;
	Bitmap* previous = nullptr;
	for (int level = 0; level <= Bitmap::kDitherLevels; level++) {
		bitmap->Clear(back);
		bitmap->FillRectDithered(all, ink, level);
		if (Count(bitmap, ink) != 16 * 12 * level / 16) {
			std::printf("FAIL: level %d painted %d pixels of %d\n", level, Count(bitmap, ink), 16 * 12);
			sFailures++;
		}
		if (previous != nullptr) {
			for (int y = 0; y < 12; y++)
				for (int x = 0; x < 16; x++)
					if (previous->GetPixel(x, y) == ink && bitmap->GetPixel(x, y) != ink)
						grows = false;
			previous->Release();
		}
		previous = bitmap->Clone();
	}
	previous->Release();
	CHECK(grows, "a higher level lost pixels of a lower one");

	// Half is a chequerboard
	bitmap->Clear(back);
	bitmap->FillRectDithered(all, ink, Bitmap::kDitherLevels / 2);
	bool chequered = true;
	for (int y = 0; y < 12; y++)
		for (int x = 0; x < 16; x++)
			chequered = chequered && (bitmap->GetPixel(x, y) == ink) == ((x + y) % 2 == 0);
	CHECK(chequered, "half the pixels are not a chequerboard");

	// Two rects side by side make the same picture as one
	Bitmap* whole = bitmap->Clone();
	bitmap->Clear(back);
	bitmap->FillRectDithered(GFX::rect(0, 0, 5, 12), ink, Bitmap::kDitherLevels / 2);
	bitmap->FillRectDithered(GFX::rect(5, 0, 11, 7), ink, Bitmap::kDitherLevels / 2);
	bitmap->FillRectDithered(GFX::rect(5, 7, 11, 5), ink, Bitmap::kDitherLevels / 2);
	bool fits = true;
	for (int y = 0; y < 12; y++)
		for (int x = 0; x < 16; x++)
			fits = fits && bitmap->GetPixel(x, y) == whole->GetPixel(x, y);
	CHECK(fits, "rects next to each other don't fit together");
	whole->Release();

	// Only inside the rect
	bitmap->Clear(back);
	bitmap->FillRectDithered(GFX::rect(4, 3, 6, 5), ink, Bitmap::kDitherLevels);
	CHECK(Count(bitmap, ink) == 30, "a full level is not the whole rect");
	CHECK(bitmap->GetPixel(3, 3) == back && bitmap->GetPixel(10, 7) == back && bitmap->GetPixel(4, 8) == back,
		"it painted outside the rect");

	// A rect that sticks out is cut to the bitmap
	bitmap->Clear(back);
	bitmap->FillRectDithered(GFX::rect(-4, -4, 8, 8), ink, Bitmap::kDitherLevels);
	bitmap->FillRectDithered(GFX::rect(12, 8, 100, 100), ink, Bitmap::kDitherLevels);
	bitmap->FillRectDithered(GFX::rect(40, 40, 5, 5), ink, Bitmap::kDitherLevels);
	CHECK(Count(bitmap, ink) == 16 + 16, "a rect that sticks out was not cut to the bitmap");

	bitmap->Release();

	if (sFailures == 0)
		std::printf("DitherTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
