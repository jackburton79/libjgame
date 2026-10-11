/*
 * GraphicsDefs.h
 *
 *  Created on: 03/apr/2015
 *      Author: stefano
 */

#pragma once

#include "SupportDefs.h"

struct SDL_Rect;

namespace GFX {

// Laid out like an SDL_Color
struct Color {
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;

	// Not set to anything, like a plain number (Color() is all zeros)
	Color() = default;
	// Opaque, unless it says otherwise
	constexpr Color(uint8 red, uint8 green, uint8 blue, uint8 alpha = 255)
		:
		r(red),
		g(green),
		b(blue),
		a(alpha)
	{
	}
};

inline bool operator==(const Color& a, const Color& b)
{
	return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

inline bool operator!=(const Color& a, const Color& b)
{
	return !(a == b);
}


// The colour with its hue turned by `hue` degrees and its saturation and
// brightness multiplied by `saturation` and `value` (0, 1, 1 leaves it as it
// is). The alpha stays.
Color Tinted(const Color& color, float hue, float saturation, float value);
// The colour `amount` of the way (0-1) from `from` to `to`
Color Blended(const Color& from, const Color& to, float amount);
// The colour `amount` of the way (0-1) to black
Color Darkened(const Color& color, float amount);


class Palette {
public:
	Palette();
	Palette(const GFX::Palette& palette);
	Palette(const GFX::Color& start, const GFX::Color& end);

	Palette& operator=(const Palette& palette);

	void Dump() const;
	void Dump(const char* fileName) const;
	Color colors[256];
};

extern bool InitializeGlobalPalettes();
extern void DestroyGlobalPalettes();

extern GFX::Palette* kPaletteRed;
extern GFX::Palette* kPaletteBlue;
extern GFX::Palette* kPaletteYellow;
extern GFX::Palette* kPaletteBlack;


// GFX::point
class point {
public:
	point();
	point(int16 x, int16 y);

	int16 x;
	int16 y;

	GFX::point& OffsetBy(const sint16& x, const sint16& y);
	GFX::point OffsetByCopy(const sint16& x, const sint16& y) const;

	void Print() const;
};

point operator+(const point& pointA, const point& pointB);
point operator-(const point& pointA, const point& pointB);


// GFX::rect
class rect {
public:
	sint16 x;
	sint16 y;
	uint16 w;
	uint16 h;

	rect();
	rect(sint16 x, sint16 y, uint16 width, uint16 height);

	GFX::point LeftTop() const;
	GFX::point Center() const;
	GFX::point RightBottom() const;

	bool Contains(const int16& x, const int16& y) const;
	bool Intersects(const GFX::rect &rectB) const;

	GFX::rect& OffsetBy(const sint16& x, const sint16& y);
	GFX::rect OffsetByCopy(const sint16& x, const sint16& y) const;
	GFX::rect& OffsetTo(const sint16& x, const sint16& y);
	GFX::rect OffsetToCopy(const sint16& x, const sint16& y) const;

	GFX::rect& CenterIn(const GFX::rect& rect);

	void Print() const;
};

void GFXRectToSDLRect(const rect* source, SDL_Rect* dest);
void SDLRectToGFXRect(const SDL_Rect* source, rect* dest);

extern const GFX::point kOrigin;

};
