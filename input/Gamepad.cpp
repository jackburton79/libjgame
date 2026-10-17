/*
 * Gamepad.cpp
 */

#include <jgame/Gamepad.h>

#include <jgame/Keyboard.h>

#include <SDL.h>

#include <map>
#include <vector>

namespace jgame {

namespace {

// What a pad can press: its buttons, and a stick or a trigger past a threshold
enum Control {
	kButtonFirst = 0,
	kButtonLast = SDL_CONTROLLER_BUTTON_MAX - 1,
	kStickUp, kStickDown, kStickLeft, kStickRight,
	kTriggerRight,
	kControlCount
};

struct Binding {
	int control;
	Keyboard::Key key;
};

const Binding kBindings[] = {
	{ SDL_CONTROLLER_BUTTON_DPAD_UP,		Keyboard::KEY_UP },
	{ SDL_CONTROLLER_BUTTON_DPAD_DOWN,		Keyboard::KEY_DOWN },
	{ SDL_CONTROLLER_BUTTON_DPAD_LEFT,		Keyboard::KEY_LEFT },
	{ SDL_CONTROLLER_BUTTON_DPAD_RIGHT,		Keyboard::KEY_RIGHT },
	{ kStickUp,								Keyboard::KEY_UP },
	{ kStickDown,							Keyboard::KEY_DOWN },
	{ kStickLeft,							Keyboard::KEY_LEFT },
	{ kStickRight,							Keyboard::KEY_RIGHT },
	{ SDL_CONTROLLER_BUTTON_A,				Keyboard::KEY_SPACE },
	{ SDL_CONTROLLER_BUTTON_A,				Keyboard::KEY_RETURN },
	{ SDL_CONTROLLER_BUTTON_B,				Keyboard::KEY_X },
	{ kTriggerRight,						Keyboard::KEY_X },
	{ SDL_CONTROLLER_BUTTON_X,				Keyboard::KEY_E },
	{ SDL_CONTROLLER_BUTTON_Y,				Keyboard::KEY_R },
	{ SDL_CONTROLLER_BUTTON_START,			Keyboard::KEY_P },
	{ SDL_CONTROLLER_BUTTON_BACK,			Keyboard::KEY_ESCAPE },
};

// An axis goes from -32768 to 32767: it presses at half and lets go at a third, so that a
// stick resting around the threshold doesn't flicker
const int kPressAt = 16384;
const int kReleaseAt = 10923;

struct Pad {
	SDL_GameController* controller = nullptr;	// null for state that came without the pad being opened
	bool down[kControlCount] = {};
};

// By instance id: a pad keeps its id as long as it is plugged in
std::map<SDL_JoystickID, Pad> sPads;
bool sInitialized = false;
// The keys that went down while an event was handled (the Keyboard is told when it is over)
std::vector<Keyboard::Key> sPressed;


void
set_control(Pad& pad, int control, bool down)
{
	if (pad.down[control] == down)
		return;
	pad.down[control] = down;
	if (!down)
		return;
	for (const Binding& binding : kBindings) {
		if (binding.control == control)
			sPressed.push_back(binding.key);
	}
}


// A pseudo button for an axis direction, with the hysteresis
void
set_axis(Pad& pad, int control, int amount)
{
	if (amount >= kPressAt)
		set_control(pad, control, true);
	else if (amount < kReleaseAt)
		set_control(pad, control, false);
}

} // namespace


/* static */
bool
Gamepad::IsConnected()
{
	for (const auto& entry : sPads) {
		if (entry.second.controller != nullptr)
			return true;
	}
	return false;
}


/* static */
bool
Gamepad::_IsKeyDown(int key)
{
	for (const auto& entry : sPads) {
		for (const Binding& binding : kBindings) {
			if (binding.key == key && entry.second.down[binding.control])
				return true;
		}
	}
	return false;
}


/* static */
void
Gamepad::_Initialize()
{
	if (sInitialized)
		return;
	sInitialized = true;
	// No pad support is not an error: the keyboard is still there
	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0)
		return;
	// The pads already plugged in come as SDL_CONTROLLERDEVICEADDED events too
}


/* static */
void
Gamepad::_HandleEvent(const union SDL_Event& event)
{
	_Handle(event);
	for (Keyboard::Key key : sPressed)
		Keyboard::_VirtualPress(key);
	sPressed.clear();
}


/* static */
void
Gamepad::_Handle(const union SDL_Event& event)
{
	switch (event.type) {
		case SDL_CONTROLLERDEVICEADDED:
		{
			SDL_GameController* controller = SDL_GameControllerOpen(event.cdevice.which);
			if (controller == nullptr)
				break;
			const SDL_JoystickID id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
			Pad& pad = sPads[id];
			if (pad.controller != nullptr)
				SDL_GameControllerClose(controller);		// opened already
			else
				pad.controller = controller;
			break;
		}
		case SDL_CONTROLLERDEVICEREMOVED:
		{
			auto found = sPads.find(event.cdevice.which);
			if (found == sPads.end())
				break;
			if (found->second.controller != nullptr)
				SDL_GameControllerClose(found->second.controller);
			sPads.erase(found);
			break;
		}
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			if (event.cbutton.button < SDL_CONTROLLER_BUTTON_MAX)
				set_control(sPads[event.cbutton.which], event.cbutton.button,
					event.type == SDL_CONTROLLERBUTTONDOWN);
			break;
		case SDL_CONTROLLERAXISMOTION:
		{
			Pad& pad = sPads[event.caxis.which];
			const int value = event.caxis.value;
			switch (event.caxis.axis) {
				case SDL_CONTROLLER_AXIS_LEFTX:
					set_axis(pad, kStickLeft, -value);
					set_axis(pad, kStickRight, value);
					break;
				case SDL_CONTROLLER_AXIS_LEFTY:
					set_axis(pad, kStickUp, -value);
					set_axis(pad, kStickDown, value);
					break;
				case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
					set_axis(pad, kTriggerRight, value);
					break;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
}

} // namespace jgame
