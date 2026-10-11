// Turning, blending and darkening colours.
#include "GraphicsDefs.h"

#include <cstdio>
#include <cstdlib>

static int sFailures = 0;

#define CHECK(cond, msg) \
	do { \
		if (!(cond)) { \
			std::printf("FAIL: %s\\n", msg); \
			sFailures++; \
		} \
	} while (0)

static bool
Is(const GFX::Color& c, int r, int g, int b, int a = 255)
{
	if (c.r == r && c.g == g && c.b == b && c.a == a)
		return true;
	std::printf("  got %d %d %d %d, expected %d %d %d %d\n", c.r, c.g, c.b, c.a, r, g, b, a);
	return false;
}


int
main()
{
	const GFX::Color red = { 255, 0, 0, 255 };
	const GFX::Color grey = { 128, 128, 128, 255 };
	const GFX::Color brick = { 200, 100, 50, 77 };

	CHECK(Is(GFX::Tinted(brick, 0, 1, 1), 200, 100, 50, 77), "a tint of nothing changed the colour");
	CHECK(Is(GFX::Tinted(red, 120, 1, 1), 0, 255, 0), "red turned 120 degrees is not green");
	CHECK(Is(GFX::Tinted(red, 240, 1, 1), 0, 0, 255), "red turned 240 degrees is not blue");
	CHECK(Is(GFX::Tinted(red, -120, 1, 1), 0, 0, 255), "red turned back 120 degrees is not blue");
	CHECK(Is(GFX::Tinted(red, 360, 1, 1), 255, 0, 0), "a full turn changed the colour");
	CHECK(Is(GFX::Tinted(red, 0, 0, 1), 255, 255, 255), "red without saturation is not white");
	CHECK(Is(GFX::Tinted(red, 0, 1, 0.5f), 128, 0, 0), "red at half brightness");
	CHECK(Is(GFX::Tinted(red, 0, 1, 0), 0, 0, 0), "no brightness is not black");
	CHECK(Is(GFX::Tinted(grey, 77, 3, 1), 128, 128, 128), "a grey got a colour");
	CHECK(Is(GFX::Tinted(grey, 0, 1, 4), 255, 255, 255), "too bright didn't stop at white");

	const GFX::Color from = { 0, 100, 200, 0 };
	const GFX::Color to = { 100, 200, 0, 200 };
	CHECK(Is(GFX::Blended(from, to, 0), 0, 100, 200, 0), "blended by nothing");
	CHECK(Is(GFX::Blended(from, to, 1), 100, 200, 0, 200), "blended all the way");
	CHECK(Is(GFX::Blended(from, to, 0.5f), 50, 150, 100, 100), "blended half way");
	CHECK(Is(GFX::Blended(from, to, -3), 0, 100, 200, 0), "blended by less than nothing");
	CHECK(Is(GFX::Blended(from, to, 7), 100, 200, 0, 200), "blended by more than all");

	CHECK(Is(GFX::Darkened(brick, 0), 200, 100, 50, 77), "darkened by nothing");
	CHECK(Is(GFX::Darkened(brick, 0.5f), 100, 50, 25, 77), "darkened by half");
	CHECK(Is(GFX::Darkened(brick, 1), 0, 0, 0, 77), "darkened all the way");

	if (sFailures == 0)
		std::printf("ColorTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
