// GraphicsEngine::SetClipping(): drawing on the screen stays inside the rect
// until the clipping is taken away again with a null rect.
#include "GraphicsEngine.h"

#include <SDL.h>

#include <cstdio>
#include <cstdlib>

static int sFailures = 0;

#define CHECK(cond, msg) \
	do { \
		if (!(cond)) { \
			std::printf("FAIL: %s\n", msg); \
			sFailures++; \
		} \
	} while (0)

int
main()
{
	SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
	if (!GraphicsEngine::Initialize()) {
		std::printf("FAIL: cannot initialize the graphics engine\n");
		return EXIT_FAILURE;
	}

	GraphicsEngine* gfx = GraphicsEngine::Get();
	gfx->SetVideoMode(64, 64, 32, GraphicsEngine::VIDEOMODE_WINDOWED);
	Bitmap* screen = gfx->ScreenBitmap();
	const uint32 black = screen->MapRGBColor(0, 0, 0);
	const uint32 white = screen->MapRGBColor(255, 255, 255);
	screen->Clear(black);

	GFX::rect clip(10, 10, 20, 20);
	gfx->SetClipping(&clip);
	screen->FillRect(GFX::rect(0, 0, 64, 64), white);
	CHECK(screen->GetPixel(15, 15) == white, "nothing was drawn inside the clipping");
	CHECK(screen->GetPixel(5, 5) == black, "drawn above and left of the clipping");
	CHECK(screen->GetPixel(40, 40) == black, "drawn below and right of the clipping");

	gfx->SetClipping(nullptr);
	screen->FillRect(GFX::rect(0, 0, 64, 64), white);
	CHECK(screen->GetPixel(5, 5) == white, "the clipping stayed after it was taken away");

	GraphicsEngine::Destroy();

	if (sFailures == 0)
		std::printf("ClippingTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
