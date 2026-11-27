// The touch controls with a real window and renderer (SDL's dummy video): fingers land on what is drawn under
// them however the screen is scaled and centred, bands at the sides included, and the controls are drawn there
#include <jgame/Events.h>
#include <jgame/GraphicsEngine.h>
#include <jgame/Keyboard.h>
#include <jgame/TouchControls.h>

#include <SDL.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace jgame;

static int sFailures = 0;

#define CHECK(cond, msg) \
	do { \
		if (!(cond)) { \
			std::printf("FAIL: %s\n", msg); \
			sFailures++; \
		} \
	} while (0)

// A finger at (x, y) pixels of a window that is w by h
static void
Touch(Uint32 type, float x, float y, int w, int h, SDL_FingerID id = 1)
{
	SDL_Event event;
	SDL_zero(event);
	event.type = type;
	event.tfinger.touchId = 1;
	event.tfinger.fingerId = id;
	event.tfinger.x = x / w;
	event.tfinger.y = y / h;
	SDL_PushEvent(&event);
}

// How many pixels of a part of the window are not black
static int
Lit(SDL_Renderer* renderer, int x, int y, int w, int h)
{
	std::vector<Uint32> pixels(w * h);
	SDL_Rect rect = { x, y, w, h };
	if (SDL_RenderReadPixels(renderer, &rect, SDL_PIXELFORMAT_ARGB8888, pixels.data(), w * 4) != 0) {
		std::printf("SDL_RenderReadPixels: %s\n", SDL_GetError());
		return -1;
	}
	int lit = 0;
	for (Uint32 pixel : pixels)
		lit += (pixel & 0x00ffffff) != 0 ? 1 : 0;
	return lit;
}

int
main()
{
	if (!GraphicsEngine::Initialize()) {
		std::printf("GraphicsEngine::Initialize failed\n");
		return EXIT_FAILURE;
	}
	GraphicsEngine* gfx = GraphicsEngine::Get();
	gfx->SetVideoMode(320, 240, 32, GraphicsEngine::VIDEOMODE_PIXELATED);
	CHECK(gfx->CanToggleFullscreen(), "a computer can toggle full screen");
	// A window wider than the screen: 1600x900 shows it 3.75 times bigger, with 200 pixels on each side
	gfx->SetWindowSize(1600, 900);
	int w = 0, h = 0;
	SDL_GetWindowSize(gfx->Window(), &w, &h);
	CHECK(w == 1600 && h == 900, "the window took its size");
	Events::Pump();
	gfx->Update();

	const GraphicsEngine::ScreenPlacement place = gfx->Placement();
	CHECK(std::fabs(place.scale - 3.75f) < 0.001f && std::fabs(place.x - 200) < 0.01f && place.y == 0, "the placement");
	// What the window shows, in the screen's pixels: 53.3 pixels of band on each side
	const GFX::rect area = TouchControls::VisibleArea();
	CHECK(area.x == -54 && area.y == 0 && area.w == 427 && area.h == 240, "the visible area");

	// A button at the bottom right of the screen
	TouchControls::AddButton(300, 220, 16, Keyboard::KEY_SPACE);
	// Where it is drawn in the window: 200 + 300 * 3.75, 220 * 3.75
	Touch(SDL_FINGERDOWN, 200 + 300 * 3.75f, 220 * 3.75f, w, h);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "the finger on the button presses it");
	Touch(SDL_FINGERUP, 200 + 300 * 3.75f, 220 * 3.75f, w, h);
	Events::Pump();
	// Half a button to the left in the window is the same on the screen: not on it any more
	Touch(SDL_FINGERDOWN, 200 + 300 * 3.75f - 30 * 3.75f, 220 * 3.75f, w, h);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "off the button");
	Touch(SDL_FINGERUP, 200 + 300 * 3.75f - 30 * 3.75f, 220 * 3.75f, w, h);
	Events::Pump();

	// A button on the band on the right, out of the screen (x past 320): where it is drawn, it answers
	TouchControls::RemoveButtons();
	TouchControls::AddButton(350, 120, 16, Keyboard::KEY_X);
	Touch(SDL_FINGERDOWN, 200 + 350 * 3.75f, 120 * 3.75f, w, h);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_X), "a button on the band at the right");
	Touch(SDL_FINGERUP, 200 + 350 * 3.75f, 120 * 3.75f, w, h);
	Events::Pump();
	// ...and one on the band on the left (x less than 0)
	TouchControls::RemoveButtons();
	TouchControls::AddButton(-30, 120, 16, Keyboard::KEY_E);
	Touch(SDL_FINGERDOWN, 200 - 30 * 3.75f, 120 * 3.75f, w, h);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_E), "a button on the band at the left");
	Touch(SDL_FINGERUP, 200 - 30 * 3.75f, 120 * 3.75f, w, h);
	Events::Pump();

	// A stick whose area is the band and the left of the screen: a finger there is its centre, and goes right
	TouchControls::RemoveButtons();
	TouchControls::SetStick(GFX::rect(-54, 0, 214, 240), 30);
	Touch(SDL_FINGERDOWN, 100, 450, w, h);
	Touch(SDL_FINGERMOTION, 100 + 60 * 3.75f, 450, w, h);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_RIGHT), "from the band onto the screen: the stick goes right");
	Touch(SDL_FINGERUP, 100 + 60 * 3.75f, 450, w, h);
	Events::Pump();

	// The default layout goes to the edges of what the window shows: the buttons on the band at the right, the
	// ring for the thumb on the one at the left
	TouchControls::SetMode(TouchControls::MODE_ON);
	TouchControls::UseDefaultLayout(320, 240);
	Events::Pump();
	gfx->Update();
	SDL_Renderer* renderer = gfx->Renderer();
	CHECK(Lit(renderer, 0, 0, 200, 900) > 0, "something drawn on the band at the left");
	CHECK(Lit(renderer, 1400, 0, 200, 900) > 0, "something drawn on the band at the right");
	CHECK(Lit(renderer, 600, 0, 400, 100) == 0, "nothing drawn at the top of the game");
	// The diamond of buttons: its right edge is near the right edge of the window
	CHECK(Lit(renderer, 1500, 600, 100, 300) > 0, "the buttons are at the edge");
	// Not drawn, off: the bands are black
	TouchControls::SetMode(TouchControls::MODE_OFF);
	gfx->Update();
	CHECK(Lit(renderer, 0, 0, 200, 900) == 0 && Lit(renderer, 1400, 0, 200, 900) == 0, "off: nothing drawn");

	// A window taller than the screen (a phone held upright): the bands are above and below
	TouchControls::SetMode(TouchControls::MODE_ON);
	SDL_SetWindowSize(gfx->Window(), 600, 1000);
	Events::Pump();
	gfx->Update();
	const GFX::rect tall = TouchControls::VisibleArea();
	CHECK(tall.x == 0 && tall.y < 0 && tall.w == 320 && tall.h > 240, "the visible area of a tall window");
	TouchControls::UseDefaultLayout(320, 240);
	gfx->Update();
	CHECK(Lit(renderer, 0, 900, 600, 100) > 0, "drawn below the game");

	GraphicsEngine::Destroy();
	if (sFailures == 0)
		std::printf("TouchScreenTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
