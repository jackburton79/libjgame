#include "Graphics.h"
#include "Polygon.h"

#include <assert.h>

#define SGN(x) ((x) > 0 ? 1 : ((x) == 0 ? 0 : (-1)))
#define ABS(x) ((x) > 0 ? (x) : (-x))

int
Graphics::DecodeRLE(const void *source, uint32 outSize,
		void *dest, uint8 compIndex)
{
	uint32 size = 0;
	uint8 *bits = (uint8*)dest;
	uint8 *srcBits = (uint8*)source;
	while (size++ < outSize) {
		uint8 byte = *srcBits++;
		if (byte == compIndex) {
			uint16 howMany = (uint16)*srcBits++;
			size += howMany;
			howMany++;
			memset(bits, byte, howMany);
			bits += howMany;
		} else {
			*bits++ = byte;
		}
	}

	size--;
	assert (size == outSize);
	return outSize;
}


/* static */
int
Graphics::Decode(const void *source, uint32 outSize, void *dest)
{
	memcpy(dest, source, outSize);
	return outSize;
}


/* static */
void
Graphics::DrawPixel(SDL_Surface *surface, uint32 x, uint32 y, uint32 color)
{
	uint32 bpp = surface->format->BytesPerPixel;
	uint32 offset = surface->pitch * y + x * bpp;

	SDL_LockSurface(surface);
	memcpy((uint8 *)surface->pixels + offset, &color, bpp);
	SDL_UnlockSurface(surface);
}


/* static */
void
Graphics::DrawLine(SDL_Surface *surface, uint32 x1, uint32 y1, uint32 x2,
		uint32 y2, uint32 color)
{
	int cycle;
	int lg_delta = x2 - x1;
	int sh_delta = y2 - y1;
	int lg_step = SGN(lg_delta);
	lg_delta = ABS(lg_delta);
	int sh_step = SGN(sh_delta);
	sh_delta = ABS(sh_delta);
	if (sh_delta < lg_delta) {
		cycle = lg_delta >> 1;
		while (x1 != x2) {
			DrawPixel(surface, x1, y1, color);
			cycle += sh_delta;
			if (cycle > lg_delta) {
				cycle -= lg_delta;
				y1 += sh_step;
			}
			x1 += lg_step;
		}
		DrawPixel(surface, x1, y1, color);
	}
	cycle = sh_delta >> 1;
	while (y1 != y2) {
		DrawPixel(surface, x1, y1, color);
		cycle += lg_delta;
		if (cycle > sh_delta) {
			cycle -= sh_delta;
			x1 += lg_step;
		}
		y1 += sh_step;
	}
	DrawPixel(surface, x1, y1, color);
}


void
Graphics::DrawPolygon(Polygon &polygon, SDL_Surface *surface)
{
	const int32 numPoints = polygon.CountPoints();
	if (numPoints <= 0)
		return;

	uint32 color = SDL_MapRGB(surface->format, 128, 0, 30);
	const point &firstPt = polygon.PointAt(0);
	for (int32 c = 0; c < numPoints - 1; c++) {
		const point &pt = polygon.PointAt(c);
		const point &nextPt = polygon.PointAt(c + 1);
		DrawLine(surface, pt.x, pt.y, nextPt.x, nextPt.y, color);
		if (c == polygon.CountPoints() - 2)
			DrawLine(surface, nextPt.x, nextPt.y, firstPt.x, firstPt.y, color);
	}
}


int
Graphics::DataToSDLSurface(const void *data, int32 width, int32 height, SDL_Surface *surface)
{
	SDL_LockSurface(surface);
	uint8 *ptr = (uint8*)data;
	uint8 *surfacePixels = (uint8*)surface->pixels;
	for (int32 y = 0; y < height; y++) {
		memcpy(surfacePixels, ptr + y * width, width);
		surfacePixels += surface->pitch;
	}
	SDL_UnlockSurface(surface);
	return 0;
}


