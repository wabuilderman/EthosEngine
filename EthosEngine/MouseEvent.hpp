#pragma once

struct MouseMovementEvent {
	int playerID = 0;
	float dX = 0.f;
	float dY = 0.f;
};

struct MouseScrollEvent {
	int playerID = 0;
	float dX = 0.f;
	float dY = 0.f;
};

struct MouseClickEvent {
	int playerID = 0;
	int buttonID = 0;
	int modifiers = 0;
};

struct MouseHoldEvent {
	int playerID = 0;
	int buttonID = 0;
	int modifiers = 0;
};

struct MouseReleaseEvent {
	int playerID = 0;
	int buttonID = 0;
	int modifiers = 0;
};

union MouseEvent {
	MouseMovementEvent movement;
	MouseScrollEvent scroll;
	MouseClickEvent click;
	MouseHoldEvent hold;
	MouseReleaseEvent release;
};