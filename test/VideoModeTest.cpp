// The window can be bigger than the screen the game draws on: the screen
// keeps the video mode's size whatever the window's.
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
	gfx->SetVideoMode(320, 240, 32,
		GraphicsEngine::VIDEOMODE_WINDOWED | GraphicsEngine::VIDEOMODE_PIXELATED);
	gfx->SetWindowSize(960, 720);

	CHECK(gfx->ScreenFrame().w == 320 && gfx->ScreenFrame().h == 240,
		"the screen took the window's size");
	CHECK(gfx->ScreenBitmap()->Width() == 320, "the screen bitmap took the window's size");

	SDL_Window* window = gfx->Window();
	int width = 0, height = 0;
	if (window != nullptr)
		SDL_GetWindowSize(window, &width, &height);
	CHECK(width == 960 && height == 720, "the window didn't take its size");

	// Drawing and showing still work
	Bitmap* screen = gfx->ScreenBitmap();
	const uint32 color = screen->MapRGBColor(10, 200, 30);
	screen->FillRect(GFX::rect(0, 0, 320, 240), color);
	gfx->Update();
	CHECK(screen->GetPixel(319, 239) == color, "the screen lost what was drawn");

	// Pixelated: what the window shows is the screen's pixels made bigger,
	// with no colours in between
	const uint32 red = screen->MapRGBColor(255, 0, 0);
	const uint32 blue = screen->MapRGBColor(0, 0, 255);
	screen->FillRect(GFX::rect(0, 0, 160, 240), red);
	screen->FillRect(GFX::rect(160, 0, 160, 240), blue);
	gfx->Update();
	SDL_Renderer* renderer = gfx->Renderer();
	uint32 row[960];
	SDL_Rect line = { 0, 360, 960, 1 };
	if (renderer == nullptr || SDL_RenderReadPixels(renderer, &line,
			SDL_PIXELFORMAT_RGB888, row, sizeof(row)) != 0) {
		CHECK(false, "cannot read back what the window shows");
	} else {
		bool sharp = true;
		for (int x = 0; x < 960; x++)
			sharp = sharp && (row[x] & 0xFFFFFF) == (x < 480 ? 0xFF0000u : 0x0000FFu);
		CHECK(sharp, "the scaled picture is not made of whole pixels");
	}

	// A new mode keeps working after the window was resized
	gfx->SetVideoMode(640, 480, 32, GraphicsEngine::VIDEOMODE_WINDOWED);
	CHECK(gfx->ScreenFrame().w == 640 && gfx->ScreenFrame().h == 480,
		"the second video mode has the wrong size");
	gfx->Update();

	GraphicsEngine::Destroy();

	if (sFailures == 0)
		std::printf("VideoModeTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
