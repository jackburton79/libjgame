/*
 * TouchControls.h
 *
 * Controls drawn on the screen, for phones and tablets: a stick and some
 * buttons, touched with the fingers. Like a game controller (see Gamepad.h)
 * they press keys: Keyboard::IsKeyDown() and Keyboard::WasKeyPressed()
 * answer for them, so a game that reads the keyboard can be played on a touch
 * screen with no other change than choosing where they are. GraphicsEngine
 * draws them, half see-through, over the window each time it updates it.
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
 * Positions and sizes are in the screen's pixels, the ones of
 * GraphicsEngine::ScreenBitmap(), but they are not kept inside the screen: a
 * phone's is wider than the game's (4:3, say), and the window has bands at
 * the sides where the game isn't. The controls can be there, out of the way,
 * under the thumbs: they are drawn and touched in the window at its own
 * resolution, and their positions can be less than 0 or more than the
 * screen's size. VisibleArea() says what the window shows in screen pixels,
 * bands included; lay the controls out on that.
 *
 * UseDefaultLayout() places the stick and buttons that stand for the keys a
 * game controller presses (see Gamepad.h) at the edges of the visible area:
 * a game that works with a pad works on a touch screen just as well.
 */
#pragma once

#include <jgame/GraphicsDefs.h>
#include <jgame/Keyboard.h>

#include <string>

union SDL_Event;
struct SDL_Renderer;

namespace jgame {

class TouchControls {
public:
	enum Mode {
		// Shown and working from the first touch of the screen (from the start,
		// on a phone or a tablet with a touch screen), hidden again when a key
		// of a keyboard or a button of a game controller is used
		MODE_AUTO,
		// Always shown and working, and the mouse works as a finger too (SDL makes
		// fingers of it): a way to try them on a computer
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

	// What the window shows, in screen pixels: the screen and the bands around
	// it. x and y are less than 0 when there are bands on the left or above;
	// the bottom right corner is (x + w, y + h). With no window yet, a screen
	// of the size UseDefaultLayout() was given, from 0, 0.
	static GFX::rect VisibleArea();

	// Places a stick on the left half of the visible area, and on the right
	// the buttons of a game controller with the keys they stand for (see
	// Gamepad.h): A (Space and Return), B (X), X (E) and Y (R), and a small
	// Start (P) at the top, all at the edges of the visible area. Replaces the
	// stick and the buttons there were. `width` and `height` are the size of
	// the screen, for when there is no window to ask yet. Call it again when
	// VisibleArea() changes (the phone is turned).
	static void UseDefaultLayout(uint16 width, uint16 height);

private:
	friend class Events;
	friend class Keyboard;
	friend class GraphicsEngine;
	static void _BeginFrame();
	// Draws the controls over the window; the screen is shown `scale` times as
	// big, with its top left corner at (x, y), in the renderer's pixels
	static void _Render(SDL_Renderer* renderer, float scale, float x, float y);
	static void _HandleEvent(const union SDL_Event& event);
	static bool _IsKeyDown(int key);
};

} // namespace jgame
