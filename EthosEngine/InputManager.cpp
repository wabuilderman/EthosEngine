#include "InputManager.hpp"
#include "MouseEvent.hpp"
#include "Message.hpp"

GamepadState InputManager::localGamepadStates[GLFW_JOYSTICK_LAST] = { 0 };
bool InputManager::isMouseLocked = false;
bool InputManager::shouldIgnoreNextClickRelease = false;
unsigned InputManager::localID = 0;
MouseState InputManager::localMouseState = {};


void InputManager::update(float dt) {
	// Update all active gamepads with the latest state information
	for (size_t i = 0; i < GLFW_JOYSTICK_LAST; ++i) {
		if (localGamepadStates[i].isActive) {
			GLFWgamepadstate state;
			if (glfwGetGamepadState((int)i, &state)) {				
				localGamepadStates[i].update(state);

				GamepadUpdateEvent e {
					localID,
					&localGamepadStates[i]
				};
				Message<GamepadUpdateEvent>::Send(e);
			}
		}
	}
}

void InputManager::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	if (isMouseLocked) {
		MouseMovementEvent mouse;
		mouse.playerID = localID;
		mouse.dX = (float) xpos;
		mouse.dY = (float) ypos;
		Message<MouseMovementEvent>::Send(mouse);
		glfwSetCursorPos(window, 0, 0);
	}
	else {
		MouseMovementEvent mouse;
		mouse.playerID = localID;
		mouse.dX = localMouseState.position.x - (float)xpos;
		mouse.dY = localMouseState.position.y - (float)ypos;
		Message<MouseMovementEvent>::Send(mouse);
		localMouseState.position.x = (float) xpos;
		localMouseState.position.y = (float) ypos;
	}
}

void InputManager::mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	MouseScrollEvent mouse;
	mouse.playerID = localID;
	mouse.dX = (float) xoffset;
	mouse.dY = (float) yoffset;
	Message<MouseScrollEvent>::Send(mouse);
}
void InputManager::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	localMouseState.buttons[button].update(action == GLFW_PRESS);
	if (localMouseState.buttons[button].isNew()) {
		if (action == GLFW_PRESS) {
			MouseClickEvent mouse;
			mouse.playerID = localID;
			mouse.buttonID = button;
			mouse.modifiers = mods;
			Message<MouseClickEvent>::Send(mouse);
		}
		else {
			if (shouldIgnoreNextClickRelease) {
				shouldIgnoreNextClickRelease = false;
			}
			else {
				MouseReleaseEvent mouse;
				mouse.playerID = localID;
				mouse.buttonID = button;
				mouse.modifiers = mods;
				Message<MouseReleaseEvent>::Send(mouse);
			}
		}
	}
	
	if (action == GLFW_PRESS) {
		MouseHoldEvent mouse;
		mouse.playerID = localID;
		mouse.buttonID = button;
		mouse.modifiers = mods;
		Message<MouseHoldEvent>::Send(mouse);
	}
}

void InputManager::joystick_callback(int jid, int eventType) {
	if (eventType == GLFW_CONNECTED) {
		localGamepadStates[jid].isActive = true;
	} else if (eventType == GLFW_DISCONNECTED) {
		localGamepadStates[jid].isActive = false;
	}
}

void InputManager::text_callback(GLFWwindow* window, unsigned int codepoint) {
	TextInputEvent textInput;
	textInput.codepoint = codepoint;
	Message<TextInputEvent>::Send(textInput);
}

void InputManager::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_REPEAT) {
		KeyHold keyHold;
		keyHold.playerID = localID;
		keyHold.keycode = key;
		keyHold.mod = mods;
		Message<KeyHold>::Send(keyHold);

	} else if (action == GLFW_PRESS) {
		KeyPress keyPress;
		keyPress.playerID = localID;
		keyPress.keycode = key;
		keyPress.mod = mods;

		if (key == GLFW_KEY_V && (mods & GLFW_MOD_CONTROL)) {
			PasteEvent pasteEvent;
			pasteEvent.playerID = localID;
			pasteEvent.text = glfwGetClipboardString(window);
			Message<PasteEvent>::Send(pasteEvent);
		}

		Message<KeyPress>::Send(keyPress);
	}
	else {
		KeyRelease keyRelease;
		keyRelease.playerID = localID;
		keyRelease.keycode = key;
		keyRelease.mod = mods;
		Message<KeyRelease>::Send(keyRelease);
	}
}