/*
 * Sprite.h
 *
 *  Created on: 29/mag/2012
 *      Author: stefano
 */

#ifndef SPRITE_H_
#define SPRITE_H_

#include "SupportDefs.h"

#include "SDL.h"

struct Color {
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;
};


struct Palette {
	Color colors[256];
};

namespace GFX {

struct rect {
	sint16 x;
	sint16 y;
	sint16 w;
	sint16 h;
};

}
class Bitmap {
public:
	Bitmap(uint16 width, uint16 height, uint16 bytesPerPixel);
	Bitmap(SDL_Surface* surface);
	~Bitmap();

	void SetColors(Color* colors, int start, int num);
	void SetPalette(const Palette& palette);
	void SetColorKey(uint8 index);
	void SetColorKey(uint8 r, uint8 g, uint8 b);
	void ClearColorKey();

	void* Pixels() const;

	uint16 Width() const;
	uint16 Height() const;
	uint16 Pitch() const;

	SDL_Surface* Surface();
private:
	SDL_Surface* fSurface;
};

#endif /* SPRITE_H_ */
