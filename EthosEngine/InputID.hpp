#pragma once

enum class InputID {
	UNKNOWN = 0,

#pragma region KeyboardKeys
	// KEYBOARD KEYS
	KEY_SPACE = ' ',
	KEY_APOSTROPHE = '\'',
	KEY_COMMA = ',',
	KEY_MINUS = '-',
	KEY_PERIOD = '.',
	KEY_SLASH = '/',

	KEY_0 = '0', KEY_1, KEY_2, KEY_3, KEY_4,
	KEY_5, KEY_6, KEY_7, NR_8, KEY_9,

	KEY_SEMICOLON = ';',
	KEY_EQUAL = '=',

	KEY_A = 'A', KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, 
	KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
	KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
	KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

	KEY_LEFT_BRACKET = '[',
	KEY_BACKSLASH = '\\',
	KEY_RIGHT_BRACKET = ']',
	KEY_GRAVE_ACCENT = '`',

	KEY_WORLD_1 = 161,
	KEY_WORLD_2 = 162,

	KEY_ESCAPE = 256,
	KEY_ENTER = 257,
	KEY_TAB = 258,
	KEY_BACKSPACE = 259,

	KEY_INSERT = 260,
	KEY_DELETE = 261,

	KEY_RIGHT = 262,
	KEY_LEFT = 263,
	KEY_DOWN = 264,
	KEY_UP = 265,

	KEY_PAGE_UP = 266,
	KEY_PAGE_DOWN = 267,

	KEY_HOME = 268,
	KEY_END = 269,

	KEY_CAPS_LOCK = 280,
	KEY_SCROLL_LOCK = 281,
	KEY_NUM_LOCK = 282,
	KEY_PRINT_SCREEN = 283,
	KEY_PAUSE = 284,

	KEY_F1 = 290, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
	KEY_F6,  KEY_F7,  KEY_F8,  KEY_F9,  KEY_F10,
	KEY_F11, KEY_F12, KEY_F13, KEY_F14, KEY_F15,
	KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20,
	KEY_F21, KEY_F22, KEY_F23, KEY_F24, KEY_F25,

	KEY_KP_0 = 320, KEY_KP_1, KEY_KP_2, KEY_KP_3, KEY_KP_4,
	KEY_KP_5, KEY_KP_6, KEY_KP_7, KEY_KP_8, KEY_KP_9,

	KEY_KP_DECIMAL = 330,
	KEY_KP_DIVIDE = 331,
	KEY_KP_MULTIPLY = 332,
	KEY_KP_SUBTRACT = 333,
	KEY_KP_ADD = 334,
	KEY_KP_ENTER = 335,
	KEY_KP_EQUAL = 336,

	KEY_LEFT_SHIFT = 340,
	KEY_LEFT_CONTROL = 341,
	KEY_LEFT_ALT = 342,
	KEY_LEFT_SUPER = 343,

	KEY_RIGHT_SHIFT = 344,
	KEY_RIGHT_CONTROL = 345,
	KEY_RIGHT_ALT = 346,
	KEY_RIGHT_SUPER = 347,

	KEY_MENU = 348,
	KEY_LAST = KEY_MENU,
#pragma endregion

#pragma region MouseButtons
	MOUSE_1, MOUSE_2, MOUSE_3, MOUSE_4,
	MOUSE_5, MOUSE_6, MOUSE_7, MOUSE_8,
	MOUSE_LEFT = MOUSE_1,
	MOUSE_RIGHT = MOUSE_2,
	MOUSE_MIDDLE = MOUSE_3,
	MOUSE_LAST = MOUSE_8,
#pragma endregion

#pragma region GamepadButtons
	GAMEPAD_A, GAMEPAD_B, GAMEPAD_X, GAMEPAD_Y,
	GAMEPAD_LEFT_BUMPER, GAMEPAD_RIGHT_BUMPER,
	GAMEPAD_BACK, GAMEPAD_START, GAMEPAD_GUIDE,
	GAMEPAD_LEFT_THUMB, GAMEPAD_RIGHT_THUMB,
	GAMEPAD_DPAD_UP, GAMEPAD_DPAD_RIGHT,
	GAMEPAD_DPAD_DOWN, GAMEPAD_DPAD_LEFT,

	GAMEPAD_CROSS = GAMEPAD_A, GAMEPAD_CIRCLE = GAMEPAD_B,
	GAMEPAD_SQUARE = GAMEPAD_X, GAMEPAD_TRIANGLE = GAMEPAD_Y,
	GAMEPAD_LAST = GAMEPAD_DPAD_LEFT,
#pragma endregion

#pragma region GamepadAxis
	GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y,
	GAMEPAD_AXIS_RIGHT_X, GAMEPAD_RIGHT_LEFT_Y,
	GAMEPAD_AXIS_LEFT_TRIGGER, GAMEPAD_AXIS_RIGHT_TRIGGER,
#pragma endregion

#pragma region MouseAxis
	MOUSE_AXIS_X, MOUSE_AXIS_Y, MOUSE_AXIS_SCROLL
#pragma endregion
};