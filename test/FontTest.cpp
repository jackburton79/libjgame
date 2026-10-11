// The built-in font: sizes, where the pixels land, scaling and the shadow.
#include "Bitmap.h"
#include "Font.h"

#include <cstdio>
#include <cstdlib>

static int sFailures = 0;

#define CHECK(cond, msg) \
	do { \
		if (!(cond)) { \
			std::printf("FAIL: %s\n", msg); \
			sFailures++; \
		} \
	} while (0)

static int
CountPixels(Bitmap* bitmap, uint32 color)
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
	CHECK(Font::TextWidth("") == 0, "an empty text has a width");
	CHECK(Font::TextWidth("A") == 3, "one glyph is not 3 wide");
	CHECK(Font::TextWidth("AB") == 7, "two glyphs are not 7 wide");
	CHECK(Font::TextWidth("AB", 3) == 21, "the width doesn't follow the scale");

	Bitmap* bitmap = new Bitmap(40, 20, 32);
	const uint32 white = bitmap->MapRGBColor(255, 255, 255);
	const uint32 black = bitmap->MapRGBColor(0, 0, 0);
	const uint32 back = bitmap->MapRGBColor(0, 0, 255);

	// "L" is a column and a base line: 7 pixels
	bitmap->Clear(back);
	Font::DrawText(bitmap, 2, 3, "L", white);
	CHECK(CountPixels(bitmap, white) == 7, "L doesn't have 7 pixels");
	CHECK(bitmap->GetPixel(2, 3) == white, "L's top left pixel is missing");
	CHECK(bitmap->GetPixel(4, 7) == white, "L's bottom right pixel is missing");
	CHECK(bitmap->GetPixel(4, 3) == back, "L painted where it is empty");

	// Lower case is drawn as upper case
	bitmap->Clear(back);
	Font::DrawText(bitmap, 2, 3, "l", white);
	CHECK(CountPixels(bitmap, white) == 7, "lower case isn't drawn as upper case");

	// Each pixel becomes a square
	bitmap->Clear(back);
	Font::DrawText(bitmap, 0, 0, "L", white, 2);
	CHECK(CountPixels(bitmap, white) == 28, "scale 2 doesn't make 4 pixels of each");

	// An unknown character leaves a blank and the text goes on after it
	bitmap->Clear(back);
	Font::DrawText(bitmap, 0, 0, "~L", white);
	CHECK(CountPixels(bitmap, white) == 7, "an unknown character painted something");
	CHECK(bitmap->GetPixel(4, 0) == white, "the text didn't advance past the unknown character");

	// Centred, over a shadow one pixel down and right
	bitmap->Clear(back);
	Font::DrawCentered(bitmap, 20, 5, "L", white);
	CHECK(bitmap->GetPixel(19, 5) == white, "the centred text is not where it should be");
	CHECK(bitmap->GetPixel(20, 6) == black, "the shadow is missing");
	CHECK(CountPixels(bitmap, white) == 7, "the shadow covered the text");

	bitmap->Release();

	if (sFailures == 0)
		std::printf("FontTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
