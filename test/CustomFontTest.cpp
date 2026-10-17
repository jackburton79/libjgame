// A font of the game's own: a class that derives from Font and a FontLoader
// that gives it out, used through the interface like the built-in one.
#include <jgame/Bitmap.h>
#include <jgame/Font.h>
#include <jgame/FontLoader.h>

#include <cstdio>
#include <cstdlib>
#include <memory>

using namespace jgame;

static int sFailures = 0;

#define CHECK(cond, msg) \
	do { \
		if (!(cond)) { \
			std::printf("FAIL: %s\n", msg); \
			sFailures++; \
		} \
	} while (0)

// Every character but the space is a solid square, 4 wide in 6
class BlockFont : public Font {
public:
	virtual int Height() const override { return 4; }
	virtual int Advance(uint8 ch) const override { return ch == ' ' ? 3 : 6; }
	virtual int GlyphWidth(uint8 ch) const override { return ch == ' ' ? 0 : 4; }
	virtual void DrawGlyph(Bitmap* target, int x, int y, uint8 ch, uint32 color,
		int scale) const override
	{
		if (ch != ' ')
			target->FillRect(GFX::rect(x, y, 4 * scale, 4 * scale), color);
	}
};

class BlockFontLoader : public FontLoader {
public:
	virtual std::unique_ptr<Font> Load(const std::string& name) override
	{
		if (name != "blocks")
			return nullptr;
		return std::make_unique<BlockFont>();
	}
};

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
	BlockFontLoader loader;
	CHECK(loader.Load("other") == nullptr, "the loader made a font it doesn't have");
	std::unique_ptr<Font> loaded = loader.Load("blocks");
	CHECK(loaded != nullptr, "the loader has no font");
	if (loaded == nullptr)
		return EXIT_FAILURE;
	const Font& font = *loaded;

	// What the base class works out from the font's own sizes
	CHECK(font.TextWidth("") == 0, "an empty text has a width");
	CHECK(font.TextWidth("A") == 4, "one glyph isn't 4 wide");
	CHECK(font.TextWidth("AB") == 10, "two glyphs aren't 6 + 4 wide");
	CHECK(font.TextWidth("A B") == 13, "the space doesn't take its 3");
	CHECK(font.TextWidth("AB", 2) == 20, "the width doesn't follow the scale");

	Bitmap* bitmap = new Bitmap(60, 20, 32);
	const uint32 white = bitmap->MapRGBColor(255, 255, 255);
	const uint32 black = bitmap->MapRGBColor(0, 0, 0);
	const uint32 back = bitmap->MapRGBColor(0, 0, 255);

	// The pen moves by the advance, a space leaves a gap
	bitmap->Clear(back);
	font.DrawText(bitmap, 1, 2, "A B", white);
	CHECK(CountPixels(bitmap, white) == 32, "two squares aren't 32 pixels");
	CHECK(bitmap->GetPixel(1, 2) == white, "the first square isn't where it should be");
	CHECK(bitmap->GetPixel(5, 2) == back, "painted between the squares");
	CHECK(bitmap->GetPixel(10, 2) == white, "the second square isn't after the space");

	bitmap->Clear(back);
	font.DrawText(bitmap, 0, 0, "A", white, 3);
	CHECK(CountPixels(bitmap, white) == 144, "scale 3 doesn't give a 12 by 12 square");

	// Centred on 30, over a shadow one pixel down and right
	bitmap->Clear(back);
	font.DrawCentered(bitmap, 30, 5, "A", white);
	CHECK(bitmap->GetPixel(28, 5) == white, "the centred text is not where it should be");
	CHECK(bitmap->GetPixel(32, 9) == black, "the shadow is missing");
	CHECK(bitmap->GetPixel(28, 5) != black, "the shadow covered the text");

	bitmap->Release();

	if (sFailures == 0)
		std::printf("CustomFontTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
