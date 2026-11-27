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
	// The label as pixels (made when it is first drawn, with the built-in font)
	std::vector<uint8> mask;
	int maskW = 0, maskH = 0;
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


// Where the finger is in the screen's pixels. SDL gives it as a fraction of the window (GraphicsEngine puts no
// logical size on the renderer, so that SDL leaves it alone): the window's pixel, less where the screen starts,
// over how big it is shown. A finger on a band at the side is out of the screen: before 0 or past its size.
void
ToScreen(float x, float y, float& screenX, float& screenY)
{
	GraphicsEngine* gfx = GraphicsEngine::Get();
	if (gfx != nullptr && gfx->Renderer() != nullptr && gfx->ScreenBitmap() != nullptr) {
		const GraphicsEngine::ScreenPlacement place = gfx->Placement();
		if (place.outputWidth > 0) {
			screenX = (x * place.outputWidth - place.x) / place.scale;
			screenY = (y * place.outputHeight - place.y) / place.scale;
			return;
		}
	}
	screenX = x * sLayoutW;
	screenY = y * sLayoutH;
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


// What is drawn over the window: everything is in the screen's pixels, and goes through here to the window's
struct Painter {
	SDL_Renderer* renderer;
	float scale, x, y;

	float X(float sx) const { return x + sx * scale; }
	float Y(float sy) const { return y + sy * scale; }

	void SetColor(int r, int g, int b, int a) const { SDL_SetRenderDrawColor(renderer, r, g, b, a); }

	// A disc, a line of the window's pixels at a time
	void Disc(float cx, float cy, float radius) const
	{
		const float px = X(cx), py = Y(cy), R = radius * scale;
		for (int dy = -(int)R; dy <= (int)R; dy++) {
			const float half = std::sqrt(std::max(0.0f, R * R - (float)dy * dy));
			SDL_RenderDrawLineF(renderer, px - half, py + dy, px + half, py + dy);
		}
	}

	// A ring `thickness` (of the screen's pixels) wide
	void Ring(float cx, float cy, float radius, float thickness) const
	{
		const float px = X(cx), py = Y(cy), R = radius * scale, inner = std::max(0.0f, R - thickness * scale);
		for (int dy = -(int)R; dy <= (int)R; dy++) {
			const float outerHalf = std::sqrt(std::max(0.0f, R * R - (float)dy * dy));
			if (std::fabs((float)dy) >= inner) {
				SDL_RenderDrawLineF(renderer, px - outerHalf, py + dy, px + outerHalf, py + dy);
				continue;
			}
			const float innerHalf = std::sqrt(std::max(0.0f, inner * inner - (float)dy * dy));
			SDL_RenderDrawLineF(renderer, px - outerHalf, py + dy, px - innerHalf, py + dy);
			SDL_RenderDrawLineF(renderer, px + innerHalf, py + dy, px + outerHalf, py + dy);
		}
	}

	// The label of a button, centred in it, as big as fits (in whole pixels of the screen)
	void Label(Button& button, bool dark) const
	{
		if (button.label.empty())
			return;
		if (button.mask.empty()) {
			const Font& font = Font::Builtin();
			button.maskW = std::max(1, font.TextWidth(button.label));
			button.maskH = std::max(1, font.Height());
			Bitmap* bitmap = new Bitmap(button.maskW, button.maskH, 32);
			bitmap->Clear(0);
			font.DrawText(bitmap, 0, 0, button.label, bitmap->MapRGBColor(255, 255, 255));
			button.mask.assign(button.maskW * button.maskH, 0);
			for (int py = 0; py < button.maskH; py++)
				for (int px = 0; px < button.maskW; px++)
					button.mask[py * button.maskW + px] = bitmap->GetPixel(px, py) != 0 ? 1 : 0;
			bitmap->Release();
		}
		int size = std::max(1, button.radius / 8);
		while (size > 1 && button.maskW * size > button.radius * 2 - 4)
			size--;
		const float left = button.x - button.maskW * size / 2.0f, top = button.y - button.maskH * size / 2.0f;
		if (dark)
			SetColor(20, 20, 30, 255);
		else
			SetColor(255, 255, 255, 235);
		for (int py = 0; py < button.maskH; py++) {
			for (int px = 0; px < button.maskW; px++) {
				if (!button.mask[py * button.maskW + px])
					continue;
				// The corners come from the same numbers, so that the squares fit with no gap
				const float x0 = X(left + px * size), y0 = Y(top + py * size);
				SDL_FRect rect = { x0, y0, X(left + (px + 1) * size) - x0, Y(top + (py + 1) * size) - y0 };
				SDL_RenderFillRectF(renderer, &rect);
			}
		}
	}
};

} // namespace


/* static */
void
TouchControls::SetMode(Mode mode)
{
	// On for good: SDL makes fingers of the mouse too
	if (mode == MODE_ON)
		SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");
	else if (sMode == MODE_ON)
		SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
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
GFX::rect
TouchControls::VisibleArea()
{
	GraphicsEngine* gfx = GraphicsEngine::Get();
	if (gfx != nullptr && gfx->Renderer() != nullptr && gfx->ScreenBitmap() != nullptr) {
		const GraphicsEngine::ScreenPlacement place = gfx->Placement();
		if (place.outputWidth > 0) {
			return GFX::rect((sint16)std::floor(-place.x / place.scale), (sint16)std::floor(-place.y / place.scale),
				(uint16)std::lround(place.outputWidth / place.scale), (uint16)std::lround(place.outputHeight / place.scale));
		}
	}
	return GFX::rect(0, 0, sLayoutW, sLayoutH);
}


/* static */
void
TouchControls::UseDefaultLayout(uint16 width, uint16 height)
{
	sLayoutW = width;
	sLayoutH = height;
	RemoveStick();
	RemoveButtons();

	const GFX::rect area = VisibleArea();
	const int right = area.x + area.w, bottom = area.y + area.h;
	// Sized on the shorter side, so that it works on a wide screen and a square one
	const int unit = std::max(4, std::min<int>(area.w, area.h) / 12);
	const int margin = unit / 2;
	// The left half of the visible area (the band at the side included), but for the top quarter, where
	// nothing is to be done
	SetStick(GFX::rect(area.x, area.y + area.h / 4, area.w / 2, area.h - area.h / 4), unit * 2);

	// A diamond of buttons, as on a pad: A at the bottom, B on the right, X on the left, Y on top
	const int r = unit;
	const int cx = right - margin - r * 3, cy = bottom - margin - r * 3;
	AddButton(cx, cy + r * 2, r, Keyboard::KEY_SPACE, Keyboard::KEY_RETURN, "A");
	AddButton(cx + r * 2, cy, r, Keyboard::KEY_X, Keyboard::KEY_NONE, "B");
	AddButton(cx - r * 2, cy, r, Keyboard::KEY_E, Keyboard::KEY_NONE, "X");
	AddButton(cx, cy - r * 2, r, Keyboard::KEY_R, Keyboard::KEY_NONE, "Y");
	// Start, small, out of the way of the thumbs
	const int small = std::max(3, unit * 2 / 3);
	AddButton(right - margin - small, area.y + margin + small, small, Keyboard::KEY_P, Keyboard::KEY_NONE, "II");
}


/* static */
void
TouchControls::_Render(SDL_Renderer* renderer, float scale, float x, float y)
{
	if (renderer == nullptr || !IsOn() || (!sHasStick && sButtons.empty()))
		return;
	const Painter paint = { renderer, scale, x, y };
	SDL_BlendMode oldBlend;
	Uint8 oldR, oldG, oldB, oldA;
	SDL_GetRenderDrawBlendMode(renderer, &oldBlend);
	SDL_GetRenderDrawColor(renderer, &oldR, &oldG, &oldB, &oldA);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	bool held[Keyboard::KEY_COUNT];
	HeldKeys(held);
	const float line = 1.0f;		// the rings are a pixel of the screen thick

	if (sHasStick) {
		Finger* finger;
		if (StickFinger(&finger)) {
			paint.SetColor(20, 20, 30, 110);
			paint.Disc(sStickX, sStickY, sStickRadius);
			paint.SetColor(150, 150, 160, 200);
			paint.Ring(sStickX, sStickY, sStickRadius, line);
			// The knob, where the finger is (no further than the edge)
			float dx = finger->x - sStickX, dy = finger->y - sStickY;
			const float length = std::hypot(dx, dy);
			if (length > sStickRadius) {
				dx = dx * sStickRadius / length;
				dy = dy * sStickRadius / length;
			}
			const float knob = std::max(2, sStickRadius / 2);
			paint.SetColor(255, 255, 255, 150);
			paint.Disc(sStickX + dx, sStickY + dy, knob);
			paint.SetColor(255, 255, 255, 230);
			paint.Ring(sStickX + dx, sStickY + dy, knob, line);
		} else {
			// Where to put the thumb: a faint ring in the lower part of the area
			const float cx = sStickArea.x + std::min<float>(sStickArea.w / 2.0f, sStickRadius * 1.5f);
			const float cy = sStickArea.y + sStickArea.h - std::min<float>(sStickArea.h / 2.0f, sStickRadius * 1.5f);
			paint.SetColor(20, 20, 30, 70);
			paint.Disc(cx, cy, sStickRadius);
			paint.SetColor(150, 150, 160, 160);
			paint.Ring(cx, cy, sStickRadius, line);
		}
	}

	for (Button& button : sButtons) {
		bool down = false;
		for (Keyboard::Key key : button.keys)
			down = down || (key > Keyboard::KEY_NONE && key < Keyboard::KEY_COUNT && held[key]);
		if (down)
			paint.SetColor(255, 255, 255, 200);
		else
			paint.SetColor(20, 20, 30, 150);
		paint.Disc(button.x, button.y, button.radius);
		if (down)
			paint.SetColor(255, 255, 255, 255);
		else
			paint.SetColor(150, 150, 160, 210);
		paint.Ring(button.x, button.y, button.radius, line);
		paint.Label(button, down);
	}

	SDL_SetRenderDrawBlendMode(renderer, oldBlend);
	SDL_SetRenderDrawColor(renderer, oldR, oldG, oldB, oldA);
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
	// A mouse that SDL makes into a finger is not a touch screen (but where the controls are on for good, it
	// is a way to try them on a computer: see SetMode())
	if (event.tfinger.touchId == SDL_MOUSE_TOUCHID && sMode != MODE_ON)
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
