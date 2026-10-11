// Making colours, and turning, blending and darkening them.
#include "GraphicsDefs.h"

#include <cstdio>
#include <cstdlib>
#include <type_traits>

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


// What Bitmap::SetColors() hands to SDL as SDL_Colors
static_assert(sizeof(GFX::Color) == 4, "a Color is not four bytes");
static_assert(std::is_trivially_copyable<GFX::Color>::value, "a Color can't be copied as bytes");
static_assert(std::is_standard_layout<GFX::Color>::value, "a Color is not laid out like a plain struct");
static_assert(std::is_trivially_default_constructible<GFX::Color>::value, "an array of Colors costs something to make");

constexpr GFX::Color kAtCompileTime(1, 2, 3);


int
main()
{
	// Three parts make an opaque colour; the fourth is the alpha
	CHECK(Is(GFX::Color(10, 20, 30), 10, 20, 30, 255), "a colour from three parts is not opaque");
	CHECK(Is(GFX::Color(10, 20, 30, 40), 10, 20, 30, 40), "a colour from four parts");
	const GFX::Color braces = { 10, 20, 30 };
	const GFX::Color bracesAlpha = { 10, 20, 30, 0 };
	CHECK(Is(braces, 10, 20, 30, 255), "a colour from three parts in braces is not opaque");
	CHECK(Is(bracesAlpha, 10, 20, 30, 0), "a colour from four parts in braces");
	CHECK(Is(GFX::Color(), 0, 0, 0, 0), "Color() is not all zeros");
	CHECK(Is(kAtCompileTime, 1, 2, 3, 255), "a colour made at compile time");
	CHECK(braces == GFX::Color(10, 20, 30) && braces != bracesAlpha, "comparing colours");
	const uint8* bytes = reinterpret_cast<const uint8*>(&bracesAlpha);
	CHECK(bytes[0] == 10 && bytes[1] == 20 && bytes[2] == 30 && bytes[3] == 0, "the parts are not in the order r, g, b, a");

	const GFX::Color red(255, 0, 0);
	const GFX::Color grey(128, 128, 128);
	const GFX::Color brick(200, 100, 50, 77);

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

	const GFX::Color from(0, 100, 200, 0);
	const GFX::Color to(100, 200, 0, 200);
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
