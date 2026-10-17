// The built-in font: sizes, where the pixels land, scaling and the shadow.
#include <jgame/Bitmap.h>
#include <jgame/Font.h>
#include <jgame/FontLoader.h>

#include <cstdio>
#include <cstdlib>

using namespace jgame;

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
	const Font& font = Font::Builtin();
	CHECK(font.Height() == 5, "the built-in font isn't 5 tall");
	CHECK(font.Advance('A') == 4 && font.GlyphWidth('A') == 3, "the built-in font's glyphs aren't 3 wide in 4");

	CHECK(font.TextWidth("") == 0, "an empty text has a width");
	CHECK(font.TextWidth("A") == 3, "one glyph is not 3 wide");
	CHECK(font.TextWidth("AB") == 7, "two glyphs are not 7 wide");
	CHECK(font.TextWidth("AB", 3) == 21, "the width doesn't follow the scale");

	Bitmap* bitmap = new Bitmap(40, 20, 32);
	const uint32 white = bitmap->MapRGBColor(255, 255, 255);
	const uint32 black = bitmap->MapRGBColor(0, 0, 0);
	const uint32 back = bitmap->MapRGBColor(0, 0, 255);

	// "L" is a column and a base line: 7 pixels
	bitmap->Clear(back);
	font.DrawText(bitmap, 2, 3, "L", white);
	CHECK(CountPixels(bitmap, white) == 7, "L doesn't have 7 pixels");
	CHECK(bitmap->GetPixel(2, 3) == white, "L's top left pixel is missing");
	CHECK(bitmap->GetPixel(4, 7) == white, "L's bottom right pixel is missing");
	CHECK(bitmap->GetPixel(4, 3) == back, "L painted where it is empty");

	// Lower case is drawn as upper case
	bitmap->Clear(back);
	font.DrawText(bitmap, 2, 3, "l", white);
	CHECK(CountPixels(bitmap, white) == 7, "lower case isn't drawn as upper case");

	// Each pixel becomes a square
	bitmap->Clear(back);
	font.DrawText(bitmap, 0, 0, "L", white, 2);
	CHECK(CountPixels(bitmap, white) == 28, "scale 2 doesn't make 4 pixels of each");

	// An unknown character leaves a blank and the text goes on after it
	bitmap->Clear(back);
	font.DrawText(bitmap, 0, 0, "~L", white);
	CHECK(CountPixels(bitmap, white) == 7, "an unknown character painted something");
	CHECK(bitmap->GetPixel(4, 0) == white, "the text didn't advance past the unknown character");

	// Centred, over a shadow one pixel down and right
	bitmap->Clear(back);
	font.DrawCentered(bitmap, 20, 5, "L", white);
	CHECK(bitmap->GetPixel(19, 5) == white, "the centred text is not where it should be");
	CHECK(bitmap->GetPixel(20, 6) == black, "the shadow is missing");
	CHECK(CountPixels(bitmap, white) == 7, "the shadow covered the text");

	bitmap->Release();

	// TruncateString(): a line off the text. Every built-in glyph is 4 wide with a
	// space of 1 after it, so n characters are 4n - 1 wide.
	{
		std::string text = "AB CD EF";		// 31 wide
		int width = -1;
		CHECK(font.TruncateString(text, 100, &width) == "AB CD EF" && text.empty() && width == 31,
			"a text that fits wasn't all taken");

		text = "AB CD EF";
		CHECK(font.TruncateString(text, 20, &width) == "AB CD" && text == "EF" && width == 19,
			"the line wasn't cut at the last space that fits");

		// Narrower than maxWidth: 19 wide doesn't fit in 19
		text = "AB CD EF";
		CHECK(font.TruncateString(text, 19, &width) == "AB" && text == "CD EF" && width == 7,
			"a line as wide as maxWidth fits");

		text = "AB CD EF";
		CHECK(font.TruncateString(text, 20, nullptr) == "AB CD", "no truncatedWidth, no line");

		text = "AB CD";
		CHECK(font.TruncateString(text, 20, &width, 2) == "AB" && text == "CD" && width == 14,
			"the scale isn't used to measure the line");

		// A word wider than the line is cut where it fits
		text = "ABCDEFGH";
		CHECK(font.TruncateString(text, 12, &width) == "ABC" && text == "DEFGH" && width == 11,
			"a long word wasn't cut where it fits");

		// Never an empty line from a text, whatever the width
		text = "AB";
		CHECK(font.TruncateString(text, 1, &width) == "A" && text == "B",
			"a line narrower than a character isn't one character");
		text = "AB";
		CHECK(font.TruncateString(text, 0) == "A" && text == "B", "maxWidth 0 isn't one character");

		text = "";
		CHECK(font.TruncateString(text, 0, &width).empty() && text.empty() && width == 0,
			"an empty text gave something");

		// All the lines of a paragraph: each fits, and together they are the text
		const std::string paragraph = "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG";
		text = paragraph;
		std::string joined;
		int lines = 0;
		while (!text.empty() && lines < 100) {
			const std::string line = font.TruncateString(text, 44, &width);
			CHECK(!line.empty() && width < 44 && width == font.TextWidth(line),
				"a line of the paragraph is empty or too wide");
			joined += (joined.empty() ? "" : " ") + line;
			lines++;
		}
		CHECK(joined == paragraph && lines == 5, "the lines don't make up the paragraph");
	}

	// The loader that has the built-in font only
	BuiltinFontLoader loader;
	std::unique_ptr<Font> loaded = loader.Load("builtin");
	CHECK(loaded != nullptr, "the built-in loader has no built-in font");
	CHECK(loaded != nullptr && loaded->TextWidth("AB") == 7, "the loaded font isn't the built-in one");
	CHECK(loader.Load("nothing like this") == nullptr, "the built-in loader found a font that isn't there");

	if (sFailures == 0)
		std::printf("FontTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
