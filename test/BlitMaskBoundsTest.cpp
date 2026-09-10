/*
 * BlitMaskBoundsTest.cpp
 *
 * Regression test for an out-of-bounds read (SEGV / heap-buffer-overflow) in
 * GraphicsEngine::BlitBitmapWithMask().
 *
 * This test drives BlitBitmapWithMask() directly with mask rectangles that sit
 * partly or entirely outside the mask.
 * It checks that nothing reads out of bounds (build it with -fsanitize=address,
 * as the Makefile rule does) and that the normal in-bounds
 * path still blits.
 */

#include "Bitmap.h"
#include "GraphicsDefs.h"
#include "GraphicsEngine.h"

#include "SDL.h"

#include <iostream>

static int sFailures = 0;

// Count non-zero pixels in a 32bpp bitmap: how much of the sprite got blitted.
static int
DrawnPixels(Bitmap* bitmap)
{
	int count = 0;
	bitmap->Lock();
	const uint8* base = (const uint8*)bitmap->Pixels();
	for (int y = 0; y < (int)bitmap->Height(); y++) {
		const uint32* row = (const uint32*)(base + y * bitmap->Pitch());
		for (int x = 0; x < (int)bitmap->Width(); x++) {
			if (row[x] != 0)
				count++;
		}
	}
	bitmap->Unlock();
	return count;
}


static int
RunCase(const char* name, int maskW, int maskH,
	int spriteW, int spriteH, int maskX, int maskY, int destX, int destY)
{
	Bitmap* sprite = new Bitmap((uint16)spriteW, (uint16)spriteH, 32);
	Bitmap* dest = new Bitmap(256, 256, 32);
	Bitmap* mask = new Bitmap((uint16)maskW, (uint16)maskH, 8);

	sprite->Clear(sprite->MapRGBColor(255, 255, 255));
	dest->Clear(0);

	// Fill the mask with "draw this pixel" so the blit path is fully exercised
	// wherever the mask access is in bounds.
	mask->Lock();
	uint8* pixels = (uint8*)mask->Pixels();
	for (int y = 0; y < maskH; y++)
		for (int x = 0; x < maskW; x++)
			pixels[y * mask->Pitch() + x] = GraphicsEngine::MASK_NO_MASK;
	mask->Unlock();

	GFX::rect destRect((sint16)destX, (sint16)destY,
		(uint16)spriteW, (uint16)spriteH);
	GFX::rect maskRect((sint16)maskX, (sint16)maskY,
		(uint16)spriteW, (uint16)spriteH);

	GraphicsEngine::BlitBitmapWithMask(sprite, NULL,
		dest, &destRect, mask, &maskRect);

	const int drawn = DrawnPixels(dest);
	std::cout << "  [ok] " << name << " (" << drawn << " px blitted)"
		<< std::endl;

	mask->Release();
	dest->Release();
	sprite->Release();

	return drawn;
}

int
main()
{
	// A video subsystem is not needed: BlitBitmapWithMask() only touches
	// software surfaces. Init with the dummy driver so this runs headless.
	SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
	SDL_Init(SDL_INIT_VIDEO);

	const int kMaskW = 64;
	const int kMaskH = 64;
	const int kSprite = 32;

	// Baseline: fully inside the mask - must still blit the whole sprite.
	if (RunCase("inside", kMaskW, kMaskH, kSprite, kSprite,
			10, 10, 10, 10) != kSprite * kSprite) {
		std::cout << "  FAIL: in-bounds blit did not draw the full sprite"
			<< std::endl;
		sFailures++;
	}

	// Sprite straddling the top-left corner: negative mask origin.
	RunCase("negative origin", kMaskW, kMaskH, kSprite, kSprite, -16, -16, 0, 0);

	// Sprite completely off the top-left.
	RunCase("fully off top-left", kMaskW, kMaskH, kSprite, kSprite,
		-64, -64, 0, 0);

	// Sprite straddling the bottom-right edge.
	RunCase("straddling bottom-right", kMaskW, kMaskH, kSprite, kSprite,
		kMaskW - 8, kMaskH - 8, 100, 100);

	// Sprite far past the bottom-right edge.
	RunCase("fully off bottom-right", kMaskW, kMaskH, kSprite, kSprite,
		kMaskW + 500, kMaskH + 500, 10, 10);

	// Large negative offsets (what a mis-repositioned party member produced):
	// area coordinates far outside the map, but still drawn at a valid
	// on-screen position.
	RunCase("large negative offset", kMaskW, kMaskH, kSprite, kSprite,
		-2000, -2000, 0, 0);

	SDL_Quit();

	if (sFailures != 0) {
		std::cout << "BlitMaskBoundsTest: FAILED (" << sFailures << ")"
			<< std::endl;
		return 1;
	}

	std::cout << "BlitMaskBoundsTest: PASSED (no out-of-bounds access)"
		<< std::endl;
	return 0;
}
