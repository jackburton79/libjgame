/*
 * TouchControls.h
 *
 * Controls drawn on the screen, for phones and tablets: a stick and some
 * buttons, touched with the fingers. Like a game controller (see Gamepad.h)
 * they press keys: Keyboard::IsKeyDown() and Keyboard::WasKeyPressed()
 * answer for them, so a game that reads the keyboard can be played on a touch
 * screen with no other change than drawing them (Draw(), once per frame,
 * before GraphicsEngine::Update()).
 *
 * The stick is floating: a finger that comes down anywhere in its area is
 * its centre, and the way the finger goes from there holds the arrow keys,
 * in eight directions (two at a time on the diagonals). A finger that goes
 * further than the stick's radius drags the centre along with it.
 *
 * A button holds its keys while a finger is on it (a finger can slide from
 * one button to the next). A finger that comes down out of the stick's area
 * and off the buttons presses the tap key, if the game set one (Return, say,
 * so that a tap goes on from a menu).
 *
 * Several fingers at once are fine: one on the stick and others on buttons.
 *
 * The positions and the sizes are in the screen's pixels, the ones of
 * GraphicsEngine::ScreenBitmap(): the fingers are brought there however the
 * screen is scaled and centred in the window.
 *
 * UseDefaultLayout() places the stick and buttons that stand for the keys a
 * game controller presses (see Gamepad.h): a game that works with a pad works
 * on a touch screen just as well.
 */
#pragma once

#include <jgame/GraphicsDefs.h>
#include <jgame/Keyboard.h>

#include <string>

union SDL_Event;

namespace jgame {

class Bitmap;

class TouchControls {
public:
	enum Mode {
		// Shown and working from the first touch of the screen (from the start,
		// on a phone or a tablet with a touch screen), hidden again when a key
		// of a keyboard or a button of a game controller is used
		MODE_AUTO,
		MODE_ON,
		MODE_OFF
	};
	static void SetMode(Mode mode);
	static Mode GetMode();
	// Whether the controls are on the screen and pressing keys now. A game can
	// ask it to tell the player what to touch instead of what to press.
	static bool IsActive();

	// The stick: touched anywhere in `area`, `radius` is how far the finger
	// goes to push it all the way. The arrow keys are held past a third of it.
	static void SetStick(const GFX::rect& area, uint16 radius);
	static void RemoveStick();
	// A round button centred on (x, y), that holds `key` (and `key2`, if it
	// isn't KEY_NONE) while it is touched. `label` is written on it, with the
	// built-in font, as big as it fits.
	static void AddButton(int16 x, int16 y, uint16 radius, Keyboard::Key key,
		Keyboard::Key key2 = Keyboard::KEY_NONE, const char* label = nullptr);
	static void RemoveButtons();
	// What a touch out of the stick's area and off the buttons presses
	// (KEY_NONE, the default, is nothing)
	static void SetTapKey(Keyboard::Key key);

	// Places a stick on the left half of a screen that is `width` by `height`
	// pixels, and on the right the buttons of a game controller with the keys
	// they stand for (see Gamepad.h): A (Space and Return), B (X), X (E) and
	// Y (R), and a small Start (P) at the top. Replaces the stick and the
	// buttons there were.
	static void UseDefaultLayout(uint16 width, uint16 height);

	// Draws the stick and the buttons, half see-through, when IsActive()
	static void Draw(Bitmap* target);

	// Where the point (x, y) of a window (0-1 across and down it, as fingers
	// are told) is on a screen of `screenWidth` by `screenHeight` pixels shown
	// in it as big as it fits, keeping its proportions, and centred (the way
	// GraphicsEngine shows it), when the window is `windowWidth` by
	// `windowHeight` pixels
	static GFX::point WindowToScreen(float x, float y, int windowWidth, int windowHeight,
		int screenWidth, int screenHeight);

private:
	friend class Events;
	friend class Keyboard;
	static void _BeginFrame();
	static void _HandleEvent(const union SDL_Event& event);
	static bool _IsKeyDown(int key);
};

} // namespace jgame
