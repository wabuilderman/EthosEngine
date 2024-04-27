#pragma once
#include "Engine.hpp"
#include "WindowWrapper.hpp"
#include <map>
#include <vector>
#include "Message.hpp"
#include "MouseEvent.hpp"

struct InputComponent {
	class Axis {
		static constexpr float deadzone = 0.2f;
		float value = 0.0f;

	public:
		operator const float& () const {
			return value;
		}
		inline bool update(const float& val) {
			float oldValue = value;
			value = (fabsf(val) > deadzone) ? val : 0.0f;
			return fabsf(oldValue - value) < FLT_EPSILON;
		}
	};

	class Button {
		bool state = false;
		bool isNewState = false;

	public:
		operator const bool& () const {
			return state;
		}

		bool isNew() const { return isNewState; }

		inline bool update(const bool& value) {
			if (isNewState = (state != value)) {
				state = value;
			}
			return isNewState;
		}
	};

	class Trigger {
		Axis axis;
	public:
		operator float() const {
			return (axis + 1.0f) / 2.0f;
		}
		inline bool update(const float& val) {
			return axis.update(val);
		}
	};

	struct Joystick {
		Axis xAxis, yAxis;
		Button hat;

		inline bool update(const float& xVal, const float& yVal, const bool& hatVal) {
			return xAxis.update(xVal) | yAxis.update(-yVal) | hat.update(hatVal);
		}
	};
	
	struct DPad {
		InputComponent::Button up, down, left, right;
		inline bool update(
			const bool& upVal, const bool& downVal,
			const bool& leftVal, const bool& rightVal
		) {
			return up.update(upVal) | down.update(downVal) | left.update(leftVal) | right.update(rightVal);
		}
	};
};

struct MouseState {
	glm::vec2 position;
	InputComponent::Button buttons[8] = {};
};

struct GamepadState {
	bool isActive = false;
	InputComponent::Trigger left_trigger, right_trigger;
	InputComponent::Joystick left_stick, right_stick;
	InputComponent::DPad dpad;

	struct FaceButtons {
		InputComponent::Button A, B, X, Y;
	} face;

	InputComponent::Button left_bumper, right_bumper;
	InputComponent::Button start, back, home;

	bool update(const GLFWgamepadstate& state) {
		bool hasChanged = false;
		hasChanged |= face.A.update(state.buttons[GLFW_GAMEPAD_BUTTON_A]);
		hasChanged |= face.B.update(state.buttons[GLFW_GAMEPAD_BUTTON_B]);
		hasChanged |= face.X.update(state.buttons[GLFW_GAMEPAD_BUTTON_X]);
		hasChanged |= face.Y.update(state.buttons[GLFW_GAMEPAD_BUTTON_Y]);

		hasChanged |= back.update(state.buttons[GLFW_GAMEPAD_BUTTON_BACK]);
		hasChanged |= start.update(state.buttons[GLFW_GAMEPAD_BUTTON_START]);
		hasChanged |= home.update(state.buttons[GLFW_GAMEPAD_BUTTON_GUIDE]);

		hasChanged |= dpad.update(
			state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP],
			state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN],
			state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT],
			state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT]
		);

		hasChanged |= left_bumper.update(state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER]);
		hasChanged |= right_bumper.update(state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER]);

		// Remapped to range 0-1
		hasChanged |= left_trigger.update(state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]);
		hasChanged |= right_trigger.update(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]);

		hasChanged |= left_stick.update(
			state.axes[GLFW_GAMEPAD_AXIS_LEFT_X],
			state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y],
			state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB]
		);

		hasChanged |= right_stick.update(
			state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X],
			state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y],
			state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB]
		);

		return hasChanged;
	}
};

struct GamepadUpdateEvent {
	unsigned playerID = 0;
	GamepadState* state = nullptr;
};

struct PasteEvent {
	unsigned playerID = 0;
	const char* text = nullptr;
};

struct TextInputEvent {
	unsigned playerID = 0;
	unsigned int codepoint = 0;
};

struct KeyInputEvent {
	unsigned playerID = 0;
	int keycode = 0;
	int mod = 0;
};

union KeyboardEvent {
	PasteEvent paste;
	TextInputEvent textInput;
	KeyInputEvent keyInput;
};

struct KeyPress : public KeyInputEvent {};
struct KeyRelease : public KeyInputEvent {};
struct KeyHold : public KeyInputEvent {};

class InputManager {
public:
	static GamepadState localGamepadStates[GLFW_JOYSTICK_LAST];
	static bool isMouseLocked;
	static bool shouldIgnoreNextClickRelease;
	static MouseState localMouseState;
	static unsigned localID;

private:
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void text_callback(GLFWwindow* window, unsigned int codepoint);
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void joystick_callback(int jid, int event);

public:
	static void lockMouse() {
		glfwSetInputMode(*Engine::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(*Engine::window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		glfwSetCursorPos(*Engine::window, 0, 0);
		isMouseLocked = true;
	}

	static void unlockMouse() {
		glfwSetInputMode(*Engine::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		isMouseLocked = false;
	}

	static void ignoreNextClickRelease() {
		shouldIgnoreNextClickRelease = true;
	}

	static void update(float dt);

	static void init() {
		glfwSetKeyCallback(*Engine::window, key_callback);
		glfwSetCharCallback(*Engine::window, text_callback);
		glfwSetCursorPosCallback(*Engine::window, cursor_position_callback);
		glfwSetMouseButtonCallback(*Engine::window, mouse_button_callback);
		glfwSetScrollCallback(*Engine::window, mouse_scroll_callback);
		glfwSetJoystickCallback(joystick_callback);

		for (size_t i = 0; i < GLFW_JOYSTICK_LAST; ++i) {
			localGamepadStates[i].isActive = glfwJoystickPresent((int)(i));
		}
	}

	static void shutdown() {
	}
};


