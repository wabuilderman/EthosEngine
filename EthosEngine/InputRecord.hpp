#pragma once
#include "InputManager.hpp"


struct InputRecord {
	enum class Type { Gamepad, Keyboard, Mouse } type;
	union {
		GamepadState gamepad;
		KeyInputEvent keyboard;
		MouseState mouse;
	} updateEvent;

	void* data;
};