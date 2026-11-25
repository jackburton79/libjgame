#include <jgame/Bitmap.h>
#include <jgame/Events.h>
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

// The screen of the test is 320x200: fingers are given in its pixels
static const int kW = 320, kH = 200;

static void
PushFinger(Uint32 type, SDL_FingerID finger, float x, float y, SDL_TouchID touch = 1)
{
	SDL_Event event;
	SDL_zero(event);
	event.type = type;
	event.tfinger.touchId = touch;
	event.tfinger.fingerId = finger;
	event.tfinger.x = x / kW;
	event.tfinger.y = y / kH;
	SDL_PushEvent(&event);
}

static void
PushKey(SDL_Scancode scancode, bool down)
{
	SDL_Event event;
	SDL_zero(event);
	event.type = down ? SDL_KEYDOWN : SDL_KEYUP;
	event.key.keysym.scancode = scancode;
	event.key.state = down ? SDL_PRESSED : SDL_RELEASED;
	SDL_PushEvent(&event);
}

int
main()
{
	if (SDL_Init(SDL_INIT_EVENTS) != 0) {
		std::printf("SDL_Init: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	// With no GraphicsEngine the fingers are on a screen of the layout's size: the default one gives it,
	// then a layout of the test's own, a stick on the left half and two buttons
	TouchControls::UseDefaultLayout(kW, kH);
	TouchControls::RemoveStick();
	TouchControls::RemoveButtons();
	TouchControls::SetStick(GFX::rect(0, 0, kW / 2, kH), 30);
	TouchControls::AddButton(280, 160, 16, Keyboard::KEY_SPACE, Keyboard::KEY_RETURN, "A");
	TouchControls::AddButton(240, 160, 16, Keyboard::KEY_X);

	Events::Pump();
	CHECK(!TouchControls::IsActive(), "not shown before a touch (no touch screen here)");

	// A button
	PushFinger(SDL_FINGERDOWN, 1, 282, 158);
	Events::Pump();
	CHECK(TouchControls::IsActive(), "shown after a touch");
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "A holds Space");
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_RETURN), "A holds Return");
	CHECK(Keyboard::WasKeyPressed(Keyboard::KEY_SPACE), "A presses Space");
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_X), "not the other button");
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "still held");
	CHECK(!Keyboard::WasKeyPressed(Keyboard::KEY_SPACE), "the press lasts one pump");
	// Sliding to the next button
	PushFinger(SDL_FINGERMOTION, 1, 242, 162);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "slid off A");
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_X), "onto the other");
	CHECK(Keyboard::WasKeyPressed(Keyboard::KEY_X), "which is pressed");
	PushFinger(SDL_FINGERUP, 1, 242, 162);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_X), "let go");

	// The stick: centred where the finger came down; a small move is nothing
	PushFinger(SDL_FINGERDOWN, 2, 80, 100);
	PushFinger(SDL_FINGERMOTION, 2, 85, 100);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_RIGHT), "a small push is nothing");
	PushFinger(SDL_FINGERMOTION, 2, 100, 100);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_RIGHT), "stick right");
	CHECK(Keyboard::WasKeyPressed(Keyboard::KEY_RIGHT), "presses Right");
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_UP) && !Keyboard::IsKeyDown(Keyboard::KEY_DOWN), "straight right");
	PushFinger(SDL_FINGERMOTION, 2, 94, 86);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_RIGHT) && Keyboard::IsKeyDown(Keyboard::KEY_UP), "a diagonal");
	CHECK(!Keyboard::WasKeyPressed(Keyboard::KEY_RIGHT), "Right was held already");
	CHECK(Keyboard::WasKeyPressed(Keyboard::KEY_UP), "Up is new");
	// A second finger on a button while the stick is held
	PushFinger(SDL_FINGERDOWN, 3, 280, 160);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_SPACE) && Keyboard::IsKeyDown(Keyboard::KEY_RIGHT), "two fingers");
	PushFinger(SDL_FINGERUP, 3, 280, 160);
	// Far to the right: the centre comes along, so going back left answers at once
	PushFinger(SDL_FINGERMOTION, 2, 200, 100);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_RIGHT) && !Keyboard::IsKeyDown(Keyboard::KEY_UP), "dragged right");
	PushFinger(SDL_FINGERMOTION, 2, 160, 100);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_LEFT), "and back left");
	PushFinger(SDL_FINGERUP, 2, 160, 100);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_LEFT) && !Keyboard::IsKeyDown(Keyboard::KEY_RIGHT), "stick let go");

	// A tap elsewhere presses the tap key, once
	PushFinger(SDL_FINGERDOWN, 4, 250, 20);
	Events::Pump();
	CHECK(!Keyboard::WasKeyPressed(Keyboard::KEY_RETURN), "no tap key yet");
	PushFinger(SDL_FINGERUP, 4, 250, 20);
	TouchControls::SetTapKey(Keyboard::KEY_RETURN);
	PushFinger(SDL_FINGERDOWN, 4, 250, 20);
	Events::Pump();
	CHECK(Keyboard::WasKeyPressed(Keyboard::KEY_RETURN), "a tap presses Return");
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_RETURN), "but doesn't hold it");
	PushFinger(SDL_FINGERUP, 4, 250, 20);
	Events::Pump();

	// A mouse made into a finger is not a touch
	PushFinger(SDL_FINGERDOWN, 5, 280, 160, SDL_MOUSE_TOUCHID);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "the mouse is no finger");

	// A keyboard hides them, and lets go of what they held; the Back key of a phone doesn't
	PushFinger(SDL_FINGERDOWN, 6, 280, 160);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "held again");
	PushKey(SDL_SCANCODE_AC_BACK, true);
	Events::Pump();
	CHECK(TouchControls::IsActive(), "Back doesn't hide them");
	CHECK(Keyboard::WasKeyPressed(Keyboard::KEY_ESCAPE), "Back is Escape");
	PushKey(SDL_SCANCODE_AC_BACK, false);
	PushKey(SDL_SCANCODE_Q, true);
	Events::Pump();
	CHECK(!TouchControls::IsActive(), "a keyboard hides them");
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "and lets go");
	PushKey(SDL_SCANCODE_Q, false);
	PushFinger(SDL_FINGERUP, 6, 280, 160);
	Events::Pump();

	// Off: no touch does anything; on: shown even before a touch
	TouchControls::SetMode(TouchControls::MODE_OFF);
	PushFinger(SDL_FINGERDOWN, 7, 280, 160);
	Events::Pump();
	CHECK(!TouchControls::IsActive() && !Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "off");
	PushFinger(SDL_FINGERUP, 7, 280, 160);
	TouchControls::SetMode(TouchControls::MODE_ON);
	Events::Pump();
	CHECK(TouchControls::IsActive(), "on");
	PushFinger(SDL_FINGERDOWN, 8, 280, 160, SDL_MOUSE_TOUCHID);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "on for good, the mouse is a finger");
	PushFinger(SDL_FINGERUP, 8, 280, 160, SDL_MOUSE_TOUCHID);
	Events::Pump();

	// Drawing: something is drawn on the buttons, and nothing far from them
	Bitmap* screen = new Bitmap(kW, kH, 32);
	screen->Clear(0);
	TouchControls::UseDefaultLayout(kW, kH);
	TouchControls::Draw(screen);
	int drawn = 0;
	for (int y = 0; y < kH; y++)
		for (int x = 0; x < kW; x++)
			drawn += screen->GetPixel(x, y) != 0 ? 1 : 0;
	CHECK(drawn > 0, "the controls are drawn");
	CHECK(screen->GetPixel(kW / 2, 2) == 0, "not all over the screen");
	screen->Release();

	SDL_Quit();
	if (sFailures == 0)
		std::printf("TouchControlsTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
