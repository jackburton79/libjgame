/*
 * GraphicsDefs.h
 *
 *  Created on: 03/apr/2015
 *      Author: stefano
 */

#ifndef GRAPHICSDEFS_H_
#define GRAPHICSDEFS_H_

#include "SupportDefs.h"

struct SDL_Rect;

namespace GFX {

struct Color {
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;
};


class Palette {
public:
	Palette();
	Palette(const GFX::Palette& palette);
	Palette(const GFX::Color& start, const GFX::Color& end);

	Palette& operator=(const Palette& palette);

	void ModColor(uint8 index, uint8 mod);
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
	GFX::point OffsetByCopy(const sint16& x, const sint16& y);

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
	GFX::rect OffsetByCopy(const sint16& x, const sint16& y);
	GFX::rect& OffsetTo(const sint16& x, const sint16& y);
	GFX::rect OffsetToCopy(const sint16& x, const sint16& y);

	void Print() const;
};

void GFXRectToSDLRect(const rect* source, SDL_Rect* dest);
void SDLRectToGFXRect(const SDL_Rect* source, rect* dest);

extern const GFX::point kOrigin;

};

#endif /* GRAPHICSDEFS_H_ */
