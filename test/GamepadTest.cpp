#include <jgame/Events.h>
#include <jgame/Gamepad.h>
#include <jgame/Keyboard.h>

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

static void
PushButton(SDL_JoystickID which, SDL_GameControllerButton button, bool down)
{
	SDL_Event event;
	SDL_zero(event);
	event.type = down ? SDL_CONTROLLERBUTTONDOWN : SDL_CONTROLLERBUTTONUP;
	event.cbutton.which = which;
	event.cbutton.button = button;
	event.cbutton.state = down ? SDL_PRESSED : SDL_RELEASED;
	SDL_PushEvent(&event);
}

static void
PushAxis(SDL_JoystickID which, SDL_GameControllerAxis axis, int value)
{
	SDL_Event event;
	SDL_zero(event);
	event.type = SDL_CONTROLLERAXISMOTION;
	event.caxis.which = which;
	event.caxis.axis = axis;
	event.caxis.value = value;
	SDL_PushEvent(&event);
}

int
main()
{
	if (SDL_Init(SDL_INIT_EVENTS) != 0) {
		std::printf("SDL_Init: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	Events::Pump();
	CHECK(!Gamepad::IsConnected(), "no pad plugged in");
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "nothing held yet");

	// A button: the keys it stands for are held, and went down once
	PushButton(0, SDL_CONTROLLER_BUTTON_A, true);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "A holds Space");
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_RETURN), "A holds Return");
	CHECK(Keyboard::WasKeyPressed(Keyboard::KEY_SPACE), "A presses Space");
	CHECK(Keyboard::WasKeyPressed(Keyboard::KEY_RETURN), "A presses Return");
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_X), "A is not X");
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "A still held");
	CHECK(!Keyboard::WasKeyPressed(Keyboard::KEY_SPACE), "the press lasts one pump");
	PushButton(0, SDL_CONTROLLER_BUTTON_A, false);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "A let go");
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_RETURN), "A let go (Return)");

	// The other buttons
	PushButton(0, SDL_CONTROLLER_BUTTON_B, true);
	PushButton(0, SDL_CONTROLLER_BUTTON_X, true);
	PushButton(0, SDL_CONTROLLER_BUTTON_Y, true);
	PushButton(0, SDL_CONTROLLER_BUTTON_START, true);
	PushButton(0, SDL_CONTROLLER_BUTTON_BACK, true);
	PushButton(0, SDL_CONTROLLER_BUTTON_DPAD_LEFT, true);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_X), "B is X");
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_E), "X is E");
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_R), "Y is R");
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_P), "Start is P");
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_ESCAPE), "Back is Escape");
	CHECK(Keyboard::WasKeyPressed(Keyboard::KEY_LEFT), "D-pad left presses Left");
	for (int button : { SDL_CONTROLLER_BUTTON_B, SDL_CONTROLLER_BUTTON_X, SDL_CONTROLLER_BUTTON_Y,
			SDL_CONTROLLER_BUTTON_START, SDL_CONTROLLER_BUTTON_BACK, SDL_CONTROLLER_BUTTON_DPAD_LEFT })
		PushButton(0, (SDL_GameControllerButton)button, false);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_LEFT), "D-pad let go");

	// The stick: past half of its travel it is a key, and it lets go only below a third
	PushAxis(0, SDL_CONTROLLER_AXIS_LEFTX, 8000);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_RIGHT), "a small push is nothing");
	PushAxis(0, SDL_CONTROLLER_AXIS_LEFTX, 20000);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_RIGHT), "stick right");
	CHECK(Keyboard::WasKeyPressed(Keyboard::KEY_RIGHT), "stick right presses Right");
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_LEFT), "not left");
	PushAxis(0, SDL_CONTROLLER_AXIS_LEFTX, 13000);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_RIGHT), "still held between the two thresholds");
	CHECK(!Keyboard::WasKeyPressed(Keyboard::KEY_RIGHT), "and not pressed again");
	PushAxis(0, SDL_CONTROLLER_AXIS_LEFTX, 0);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_RIGHT), "stick back in the middle");
	PushAxis(0, SDL_CONTROLLER_AXIS_LEFTX, -30000);
	PushAxis(0, SDL_CONTROLLER_AXIS_LEFTY, -30000);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_LEFT), "stick left");
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_UP), "stick up");
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_DOWN), "not down");
	PushAxis(0, SDL_CONTROLLER_AXIS_LEFTX, 0);
	PushAxis(0, SDL_CONTROLLER_AXIS_LEFTY, 30000);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_LEFT) && !Keyboard::IsKeyDown(Keyboard::KEY_UP), "stick moved on");
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_DOWN), "stick down");
	PushAxis(0, SDL_CONTROLLER_AXIS_LEFTY, 0);

	// The right trigger is X, the left one nothing
	PushAxis(0, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 30000);
	PushAxis(0, SDL_CONTROLLER_AXIS_TRIGGERLEFT, 30000);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_X), "right trigger is X");
	PushAxis(0, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 0);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_X), "right trigger let go");

	// Two pads: a key is held while either holds it
	PushButton(0, SDL_CONTROLLER_BUTTON_A, true);
	PushButton(1, SDL_CONTROLLER_BUTTON_A, true);
	Events::Pump();
	PushButton(0, SDL_CONTROLLER_BUTTON_A, false);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "the other pad still holds it");
	PushButton(1, SDL_CONTROLLER_BUTTON_A, false);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "both let go");

	// A pad taken away lets go of what it held
	PushButton(2, SDL_CONTROLLER_BUTTON_A, true);
	Events::Pump();
	CHECK(Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "held by pad 2");
	SDL_Event removed;
	SDL_zero(removed);
	removed.type = SDL_CONTROLLERDEVICEREMOVED;
	removed.cdevice.which = 2;
	SDL_PushEvent(&removed);
	Events::Pump();
	CHECK(!Keyboard::IsKeyDown(Keyboard::KEY_SPACE), "unplugged");

	SDL_Quit();
	if (sFailures == 0)
		std::printf("GamepadTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
