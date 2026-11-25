// The touch controls with a real window and renderer (SDL's dummy video): a finger lands on what is drawn
// under it, however the screen is scaled and centred in the window
#include <jgame/Events.h>
#include <jgame/GraphicsEngine.h>
#include <jgame/Keyboard.h>
#include <jgame/TouchControls.h>

#include <SDL.h>

#include <cstdio>
#include <cstdlib>

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
Touch(Uint32 type, float x, float y, int w, int h)
{
	SDL_Event event;
	SDL_zero(event);
	event.type = type;
	event.tfinger.touchId = 1;
	event.tfinger.fingerId = 1;
	event.tfinger.x = x / w;
	event.tfinger.y = y / h;
	SDL_PushEvent(&event);
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
	// A stick on the left half: a finger in the left band is on the screen's left edge, and holds it
	TouchControls::SetStick(GFX::rect(0, 0, 160, 240), 30);
	Touch(SDL_FINGERDOWN, 100, 450, w, h);
	Touch(SDL_FINGERMOTION, 100 + 60 * 3.75f, 450, w, h);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_RIGHT), "from the band onto the screen: the stick goes right");
	Touch(SDL_FINGERUP, 100 + 60 * 3.75f, 450, w, h);
	Events::Pump();

	GraphicsEngine::Destroy();
	if (sFailures == 0)
		std::printf("TouchScreenTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
