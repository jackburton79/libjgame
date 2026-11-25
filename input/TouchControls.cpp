/*
 * TouchControls.cpp
 */

#include <jgame/TouchControls.h>

#include <jgame/Bitmap.h>
#include <jgame/Font.h>
#include <jgame/GraphicsEngine.h>
#include <jgame/Platform.h>

#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

namespace jgame {

namespace {

struct Button {
	int x;
	int y;
	int radius;
	Keyboard::Key keys[2];
	std::string label;
};

// What a finger is doing: on the stick, on a button (or on none, sliding about
// after it came down on one), or nothing (a tap, done when it came down)
struct Finger {
	enum What { kNothing, kStick, kButton } what = kNothing;
	int button = -1;			// kButton: the one under the finger, -1 if none
	float x = 0, y = 0;			// where it is, in the screen's pixels
};

TouchControls::Mode sMode = TouchControls::MODE_AUTO;
bool sActive = false;			// MODE_AUTO: shown, since a touch
bool sChecked = false;			// MODE_AUTO: whether a touch screen was looked for at the start

bool sHasStick = false;
GFX::rect sStickArea;
int sStickRadius = 0;
float sStickX = 0, sStickY = 0;	// the stick's centre, where its finger came down (dragged along)
SDL_FingerID sStickFinger = 0;

std::vector<Button> sButtons;
Keyboard::Key sTapKey = Keyboard::KEY_NONE;

// The size of the screen the layout is for, when there is no GraphicsEngine to ask
int sLayoutW = 0, sLayoutH = 0;

std::map<SDL_FingerID, Finger> sFingers;

// A button is a bit bigger to the finger than it looks, for fingers are not precise
const float kButtonSlack = 1.25f;
// The stick holds nothing until the finger has moved this much of its radius
const float kStickDeadZone = 0.33f;
// sin(22.5 degrees): past it the other axis of the stick counts too (a diagonal)
const float kDiagonal = 0.38f;


bool
IsOn()
{
	return sMode == TouchControls::MODE_ON || (sMode == TouchControls::MODE_AUTO && sActive);
}


bool
StickFinger(Finger** finger)
{
	auto found = sFingers.find(sStickFinger);
	if (found == sFingers.end() || found->second.what != Finger::kStick)
		return false;
	*finger = &found->second;
	return true;
}


int
ButtonAt(float x, float y)
{
	// The nearest of those the point is on, when they are close together
	int best = -1;
	float bestDistance = 0;
	for (size_t i = 0; i < sButtons.size(); i++) {
		const Button& button = sButtons[i];
		const float distance = std::hypot(x - button.x, y - button.y);
		if (distance <= button.radius * kButtonSlack && (best < 0 || distance < bestDistance)) {
			best = (int)i;
			bestDistance = distance;
		}
	}
	return best;
}


// WindowToScreen(), without rounding
bool
MapToScreen(float x, float y, int windowW, int windowH, int screenW, int screenH,
	float& screenX, float& screenY)
{
	if (windowW <= 0 || windowH <= 0 || screenW <= 0 || screenH <= 0)
		return false;
	const float scale = std::min((float)windowW / screenW, (float)windowH / screenH);
	screenX = (x * windowW - (windowW - screenW * scale) / 2) / scale;
	screenY = (y * windowH - (windowH - screenH * scale) / 2) / scale;
	return true;
}


// Where the finger is on the screen, from the point of the window that SDL gives
void
ToScreen(float x, float y, float& screenX, float& screenY)
{
	int windowW = sLayoutW, windowH = sLayoutH;
	int screenW = sLayoutW, screenH = sLayoutH;
	GraphicsEngine* gfx = GraphicsEngine::Get();
	if (gfx != nullptr && gfx->Renderer() != nullptr && gfx->ScreenBitmap() != nullptr
		&& SDL_GetRendererOutputSize(gfx->Renderer(), &windowW, &windowH) == 0) {
		screenW = gfx->ScreenBitmap()->Width();
		screenH = gfx->ScreenBitmap()->Height();
	}
	if (!MapToScreen(x, y, windowW, windowH, screenW, screenH, screenX, screenY))
		screenX = screenY = -1;
}


// Which keys the stick holds now
void
StickKeys(bool held[Keyboard::KEY_COUNT])
{
	Finger* finger;
	if (!sHasStick || !StickFinger(&finger))
		return;
	const float dx = finger->x - sStickX, dy = finger->y - sStickY;
	const float length = std::hypot(dx, dy);
	if (length < sStickRadius * kStickDeadZone || length == 0)
		return;
	if (dx > length * kDiagonal)
		held[Keyboard::KEY_RIGHT] = true;
	else if (dx < -length * kDiagonal)
		held[Keyboard::KEY_LEFT] = true;
	if (dy > length * kDiagonal)
		held[Keyboard::KEY_DOWN] = true;
	else if (dy < -length * kDiagonal)
		held[Keyboard::KEY_UP] = true;
}


void
HeldKeys(bool held[Keyboard::KEY_COUNT])
{
	for (int i = 0; i < Keyboard::KEY_COUNT; i++)
		held[i] = false;
	if (!IsOn())
		return;
	StickKeys(held);
	for (const auto& entry : sFingers) {
		const Finger& finger = entry.second;
		if (finger.what != Finger::kButton || finger.button < 0 || finger.button >= (int)sButtons.size())
			continue;
		for (Keyboard::Key key : sButtons[finger.button].keys) {
			if (key > Keyboard::KEY_NONE && key < Keyboard::KEY_COUNT)
				held[key] = true;
		}
	}
}


void
FingerDown(SDL_FingerID id, float x, float y, std::vector<Keyboard::Key>& taps)
{
	Finger finger;
	finger.x = x;
	finger.y = y;
	Finger* stick;
	const int button = ButtonAt(x, y);
	if (button >= 0) {
		finger.what = Finger::kButton;
		finger.button = button;
	} else if (sHasStick && sStickArea.Contains((int16)x, (int16)y) && !StickFinger(&stick)) {
		finger.what = Finger::kStick;
		sStickFinger = id;
		sStickX = x;
		sStickY = y;
	} else if (sTapKey != Keyboard::KEY_NONE)
		taps.push_back(sTapKey);
	sFingers[id] = finger;
}


void
FingerMoved(SDL_FingerID id, float x, float y)
{
	auto found = sFingers.find(id);
	if (found == sFingers.end())
		return;
	Finger& finger = found->second;
	finger.x = x;
	finger.y = y;
	if (finger.what == Finger::kButton)
		finger.button = ButtonAt(x, y);
	else if (finger.what == Finger::kStick) {
		// Past the edge the centre comes along, so that going back the other way answers at once
		const float dx = x - sStickX, dy = y - sStickY;
		const float length = std::hypot(dx, dy);
		if (length > sStickRadius && length > 0) {
			sStickX = x - dx * sStickRadius / length;
			sStickY = y - dy * sStickRadius / length;
		}
	}
}


void
ReleaseAll()
{
	sFingers.clear();
}


void
FillCircleDithered(Bitmap* target, int cx, int cy, int radius, uint32 color, uint8 level)
{
	for (int dy = -radius; dy <= radius; dy++) {
		const int half = (int)std::sqrt((float)(radius * radius - dy * dy));
		target->FillRectDithered(GFX::rect(cx - half, cy + dy, 2 * half + 1, 1), color, level);
	}
}

} // namespace


/* static */
void
TouchControls::SetMode(Mode mode)
{
	sMode = mode;
	if (mode == MODE_OFF)
		ReleaseAll();
}


/* static */
TouchControls::Mode
TouchControls::GetMode()
{
	return sMode;
}


/* static */
bool
TouchControls::IsActive()
{
	return IsOn();
}


/* static */
void
TouchControls::SetStick(const GFX::rect& area, uint16 radius)
{
	sHasStick = true;
	sStickArea = area;
	sStickRadius = std::max<int>(1, radius);
}


/* static */
void
TouchControls::RemoveStick()
{
	sHasStick = false;
	for (auto& entry : sFingers) {
		if (entry.second.what == Finger::kStick)
			entry.second.what = Finger::kNothing;
	}
}


/* static */
void
TouchControls::AddButton(int16 x, int16 y, uint16 radius, Keyboard::Key key,
	Keyboard::Key key2, const char* label)
{
	Button button;
	button.x = x;
	button.y = y;
	button.radius = std::max<int>(1, radius);
	button.keys[0] = key;
	button.keys[1] = key2;
	button.label = label != nullptr ? label : "";
	sButtons.push_back(button);
}


/* static */
void
TouchControls::RemoveButtons()
{
	sButtons.clear();
	for (auto& entry : sFingers) {
		if (entry.second.what == Finger::kButton)
			entry.second.button = -1;
	}
}


/* static */
void
TouchControls::SetTapKey(Keyboard::Key key)
{
	sTapKey = key;
}


/* static */
void
TouchControls::UseDefaultLayout(uint16 width, uint16 height)
{
	sLayoutW = width;
	sLayoutH = height;
	RemoveStick();
	RemoveButtons();

	// Sized on the shorter side, so that it works on a wide screen and a square one
	const int unit = std::max(4, std::min<int>(width, height) / 12);
	const int margin = unit / 2;
	SetStick(GFX::rect(0, height / 4, width / 2, height - height / 4), unit * 2);

	// A diamond of buttons, as on a pad: A at the bottom, B on the right, X on the left, Y on top
	const int r = unit;
	const int cx = width - margin - r * 3, cy = height - margin - r * 3;
	AddButton(cx, cy + r * 2, r, Keyboard::KEY_SPACE, Keyboard::KEY_RETURN, "A");
	AddButton(cx + r * 2, cy, r, Keyboard::KEY_X, Keyboard::KEY_NONE, "B");
	AddButton(cx - r * 2, cy, r, Keyboard::KEY_E, Keyboard::KEY_NONE, "X");
	AddButton(cx, cy - r * 2, r, Keyboard::KEY_R, Keyboard::KEY_NONE, "Y");
	// Start, small, out of the way of the thumbs
	const int small = std::max(3, unit * 2 / 3);
	AddButton(width - margin - small, margin + small, small, Keyboard::KEY_P, Keyboard::KEY_NONE, "II");
}


/* static */
void
TouchControls::Draw(Bitmap* target)
{
	if (target == nullptr || !IsOn())
		return;
	const uint32 white = target->MapRGBColor(255, 255, 255);
	const uint32 grey = target->MapRGBColor(150, 150, 160);
	const uint32 dark = target->MapRGBColor(20, 20, 30);

	bool held[Keyboard::KEY_COUNT];
	HeldKeys(held);

	if (sHasStick) {
		Finger* finger;
		if (StickFinger(&finger)) {
			FillCircleDithered(target, (int)sStickX, (int)sStickY, sStickRadius, dark, 6);
			target->StrokeCircle((int16)sStickX, (int16)sStickY, sStickRadius, grey);
			// The knob, where the finger is (no further than the edge)
			float dx = finger->x - sStickX, dy = finger->y - sStickY;
			const float length = std::hypot(dx, dy);
			if (length > sStickRadius) {
				dx = dx * sStickRadius / length;
				dy = dy * sStickRadius / length;
			}
			const int knob = std::max(2, sStickRadius / 2);
			FillCircleDithered(target, (int)(sStickX + dx), (int)(sStickY + dy), knob, white, 8);
			target->StrokeCircle((int16)(sStickX + dx), (int16)(sStickY + dy), knob, white);
		} else {
			// Where to put the thumb: a faint ring in the lower part of the area
			const int x = sStickArea.x + std::min<int>(sStickArea.w / 2, sStickRadius * 2);
			const int y = sStickArea.y + sStickArea.h - std::min<int>(sStickArea.h / 2, sStickRadius * 2);
			FillCircleDithered(target, x, y, sStickRadius, dark, 4);
			target->StrokeCircle(x, y, sStickRadius, grey);
		}
	}

	for (const Button& button : sButtons) {
		bool down = false;
		for (Keyboard::Key key : button.keys)
			down = down || (key > Keyboard::KEY_NONE && key < Keyboard::KEY_COUNT && held[key]);
		FillCircleDithered(target, button.x, button.y, button.radius, down ? white : dark, down ? 8 : 6);
		target->StrokeCircle(button.x, button.y, button.radius, down ? white : grey);
		if (!button.label.empty()) {
			const Font& font = Font::Builtin();
			const int scale = std::max(1, button.radius / 6);
			font.DrawCentered(target, button.x, button.y - font.Height() * scale / 2, button.label,
				down ? dark : white, scale);
		}
	}
}


/* static */
GFX::point
TouchControls::WindowToScreen(float x, float y, int windowWidth, int windowHeight,
	int screenWidth, int screenHeight)
{
	float screenX, screenY;
	if (!MapToScreen(x, y, windowWidth, windowHeight, screenWidth, screenHeight, screenX, screenY))
		return GFX::point(-1, -1);
	return GFX::point((int16)std::floor(screenX), (int16)std::floor(screenY));
}


/* static */
void
TouchControls::_BeginFrame()
{
	// On a phone or a tablet they are there from the start: there may be nothing else to play with
	if (!sChecked) {
		sChecked = true;
		if (sMode == MODE_AUTO && Platform::IsMobile() && SDL_WasInit(SDL_INIT_VIDEO)
			&& Platform::HasTouchScreen())
			sActive = true;
	}
}


/* static */
void
TouchControls::_HandleEvent(const union SDL_Event& event)
{
	switch (event.type) {
		case SDL_FINGERDOWN:
		case SDL_FINGERMOTION:
		case SDL_FINGERUP:
			break;
		case SDL_KEYDOWN:
			// A keyboard (the Back key of a phone doesn't count: every phone has it)
			if (event.key.keysym.scancode != SDL_SCANCODE_AC_BACK && sMode == MODE_AUTO && sActive) {
				sActive = false;
				ReleaseAll();
			}
			return;
		case SDL_CONTROLLERBUTTONDOWN:
			if (sMode == MODE_AUTO && sActive) {
				sActive = false;
				ReleaseAll();
			}
			return;
		default:
			return;
	}
	if (sMode == MODE_OFF)
		return;
	// A mouse that SDL makes into a finger is not a touch screen
	if (event.tfinger.touchId == SDL_MOUSE_TOUCHID)
		return;

	bool before[Keyboard::KEY_COUNT];
	HeldKeys(before);
	std::vector<Keyboard::Key> taps;
	if (event.type == SDL_FINGERDOWN && sMode == MODE_AUTO && !sActive) {
		sActive = true;
		ReleaseAll();
	}

	float x, y;
	ToScreen(event.tfinger.x, event.tfinger.y, x, y);
	if (event.type == SDL_FINGERDOWN)
		FingerDown(event.tfinger.fingerId, x, y, taps);
	else if (event.type == SDL_FINGERMOTION)
		FingerMoved(event.tfinger.fingerId, x, y);
	else
		sFingers.erase(event.tfinger.fingerId);

	// The keys that went down with this, as Keyboard::WasKeyPressed() tells
	bool after[Keyboard::KEY_COUNT];
	HeldKeys(after);
	for (int i = Keyboard::KEY_NONE + 1; i < Keyboard::KEY_COUNT; i++) {
		if (after[i] && !before[i])
			Keyboard::_VirtualPress((Keyboard::Key)i);
	}
	for (Keyboard::Key key : taps)
		Keyboard::_VirtualPress(key);
}


/* static */
bool
TouchControls::_IsKeyDown(int key)
{
	if (key <= Keyboard::KEY_NONE || key >= Keyboard::KEY_COUNT || sFingers.empty())
		return false;
	bool held[Keyboard::KEY_COUNT];
	HeldKeys(held);
	return held[key];
}

} // namespace jgame
