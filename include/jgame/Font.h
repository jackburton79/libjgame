/*
 * Font.h
 *
 * What the library needs from a font to put text on a Bitmap, as an interface
 * (Font), so a game can draw with a font of its own: derive from it, say how
 * wide and tall the characters are and how to draw one, and the text
 * methods (TextWidth(), DrawText(), DrawCentered()) work with it. Where fonts
 * come from is up to a FontLoader (see FontLoader.h).
 *
 * Text is a sequence of bytes, a character each. A font that wants more (UTF-8,
 * kerning...) overrides TextWidth() and DrawText() too.
 *
 * BuiltinFont is a tiny 3x5 pixel font (upper case, digits, a little
 * punctuation) that needs no font file: lower case is drawn as upper case, and
 * characters it doesn't have as blanks.
 */
#pragma once

#include <jgame/Bitmap.h>

#include <string>

namespace jgame {

class Font {
public:
	virtual ~Font();

	// All the sizes are in the font's own pixels: the text methods multiply
	// them by `scale`, and so does DrawGlyph().

	// How tall a line of text is
	virtual int Height() const = 0;
	// How far the pen moves to the right after `ch`: its width and the space
	// that follows
	virtual int Advance(uint8 ch) const = 0;
	// How wide `ch` is, without the space after it
	virtual int GlyphWidth(uint8 ch) const = 0;
	// Draws `ch` with its top left corner at (x, y), each pixel of the font a
	// square of `scale` by `scale`. Draws nothing for a character the font
	// doesn't have.
	virtual void DrawGlyph(Bitmap* target, int x, int y, uint8 ch, uint32 color,
		int scale = 1) const = 0;

	// The width of `text`: the advance of every character but the last, and
	// the width of the last
	virtual int TextWidth(const std::string& text, int scale = 1) const;
	// The characters one after the other, the pen moving by their advance
	virtual void DrawText(Bitmap* target, int x, int y, const std::string& text,
		uint32 color, int scale = 1) const;
	// Centred on `centreX`, with a dark drop shadow for legibility
	void DrawCentered(Bitmap* target, int centreX, int y, const std::string& text,
		uint32 color, int scale = 1) const;

	// The built-in font
	static const Font& Builtin();
};


class BuiltinFont final : public Font {
public:
	static constexpr int kGlyphW = 3;
	static constexpr int kGlyphH = 5;
	static constexpr int kAdvance = 4;

	virtual int Height() const override;
	virtual int Advance(uint8 ch) const override;
	virtual int GlyphWidth(uint8 ch) const override;
	virtual void DrawGlyph(Bitmap* target, int x, int y, uint8 ch, uint32 color,
		int scale = 1) const override;
};

} // namespace jgame
