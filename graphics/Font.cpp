#include <jgame/Font.h>

#include <cctype>
#include <cstring>

namespace jgame {

namespace {

struct Glyph {
	char ch;
	const char* rows[5];
};

const Glyph kGlyphs[] = {
	{ 'A', { "010", "101", "111", "101", "101" } },
	{ 'B', { "110", "101", "110", "101", "110" } },
	{ 'C', { "011", "100", "100", "100", "011" } },
	{ 'D', { "110", "101", "101", "101", "110" } },
	{ 'E', { "111", "100", "110", "100", "111" } },
	{ 'F', { "111", "100", "110", "100", "100" } },
	{ 'G', { "011", "100", "101", "101", "011" } },
	{ 'H', { "101", "101", "111", "101", "101" } },
	{ 'I', { "111", "010", "010", "010", "111" } },
	{ 'J', { "001", "001", "001", "101", "010" } },
	{ 'K', { "101", "101", "110", "101", "101" } },
	{ 'L', { "100", "100", "100", "100", "111" } },
	{ 'M', { "101", "111", "111", "101", "101" } },
	{ 'N', { "110", "101", "101", "101", "101" } },
	{ 'O', { "010", "101", "101", "101", "010" } },
	{ 'P', { "110", "101", "110", "100", "100" } },
	{ 'Q', { "010", "101", "101", "111", "011" } },
	{ 'R', { "110", "101", "110", "101", "101" } },
	{ 'S', { "011", "100", "010", "001", "110" } },
	{ 'T', { "111", "010", "010", "010", "010" } },
	{ 'U', { "101", "101", "101", "101", "111" } },
	{ 'V', { "101", "101", "101", "101", "010" } },
	{ 'W', { "101", "101", "111", "111", "101" } },
	{ 'X', { "101", "101", "010", "101", "101" } },
	{ 'Y', { "101", "101", "010", "010", "010" } },
	{ 'Z', { "111", "001", "010", "100", "111" } },
	{ '0', { "111", "101", "101", "101", "111" } },
	{ '1', { "010", "110", "010", "010", "111" } },
	{ '2', { "110", "001", "010", "100", "111" } },
	{ '3', { "110", "001", "010", "001", "110" } },
	{ '4', { "101", "101", "111", "001", "001" } },
	{ '5', { "111", "100", "110", "001", "110" } },
	{ '6', { "011", "100", "111", "101", "111" } },
	{ '7', { "111", "001", "010", "010", "010" } },
	{ '8', { "111", "101", "111", "101", "111" } },
	{ '9', { "111", "101", "111", "001", "110" } },
	{ ':', { "000", "010", "000", "010", "000" } },
	{ '.', { "000", "000", "000", "000", "010" } },
	{ ',', { "000", "000", "000", "010", "100" } },
	{ '-', { "000", "000", "111", "000", "000" } },
	{ '+', { "000", "010", "111", "010", "000" } },
	{ '/', { "001", "001", "010", "100", "100" } },
	{ '!', { "010", "010", "010", "000", "010" } },
	{ '?', { "110", "001", "010", "000", "010" } },
	{ '%', { "101", "001", "010", "100", "101" } },
	{ '(', { "010", "100", "100", "100", "010" } },
	{ ')', { "010", "001", "001", "001", "010" } },
	{ '<', { "001", "010", "100", "010", "001" } },
	{ '>', { "100", "010", "001", "010", "100" } },
};

const Glyph*
Find(char c)
{
	c = (char)std::toupper((unsigned char)c);
	for (const Glyph& g : kGlyphs)
		if (g.ch == c)
			return &g;
	return nullptr;
}

} // namespace


Font::~Font()
{
}


int
Font::TextWidth(const std::string& text, int scale) const
{
	if (text.empty())
		return 0;
	int width = 0;
	for (size_t i = 0; i + 1 < text.size(); i++)
		width += Advance((uint8)text[i]);
	width += GlyphWidth((uint8)text[text.size() - 1]);
	return width * scale;
}


void
Font::DrawText(Bitmap* target, int x, int y, const std::string& text, uint32 color,
	int scale) const
{
	for (char c : text) {
		DrawGlyph(target, x, y, (uint8)c, color, scale);
		x += Advance((uint8)c) * scale;
	}
}


void
Font::DrawCentered(Bitmap* target, int centreX, int y, const std::string& text,
	uint32 color, int scale) const
{
	int x = centreX - TextWidth(text, scale) / 2;
	DrawText(target, x + scale, y + scale, text, target->MapRGBColor(0, 0, 0), scale);
	DrawText(target, x, y, text, color, scale);
}


/* static */
const Font&
Font::Builtin()
{
	static const BuiltinFont font;
	return font;
}


int
BuiltinFont::Height() const
{
	return kGlyphH;
}


int
BuiltinFont::Advance(uint8) const
{
	return kAdvance;
}


int
BuiltinFont::GlyphWidth(uint8) const
{
	return kGlyphW;
}


void
BuiltinFont::DrawGlyph(Bitmap* target, int x, int y, uint8 ch, uint32 color,
	int scale) const
{
	const Glyph* g = Find((char)ch);
	if (g == nullptr)
		return;
	for (int row = 0; row < kGlyphH; row++) {
		for (int col = 0; col < kGlyphW; col++) {
			if (g->rows[row][col] == '1')
				target->FillRect(GFX::rect(x + col * scale, y + row * scale,
					scale, scale), color);
		}
	}
}

} // namespace jgame
