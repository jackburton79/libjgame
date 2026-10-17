/*
 * Font.h
 *
 * A tiny built-in 3x5 pixel font (upper case, digits, a little punctuation):
 * text without any font file. Lower case is drawn as upper case, and
 * characters it doesn't have as blanks.
 */
#pragma once

#include <jgame/Bitmap.h>

#include <string>

namespace jgame {

namespace Font {

constexpr int kGlyphW = 3;
constexpr int kGlyphH = 5;
constexpr int kAdvance = 4;

int TextWidth(const std::string& text, int scale = 1);
void DrawText(Bitmap* target, int x, int y, const std::string& text, uint32 color,
	int scale = 1);
// Centred on `centreX`, with a dark drop shadow for legibility
void DrawCentered(Bitmap* target, int centreX, int y, const std::string& text,
	uint32 color, int scale = 1);

} // namespace Font

} // namespace jgame
