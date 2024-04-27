#pragma once
#include "UIComponent.hpp"
#include "InputManager.hpp"

class UIButton : public UIComponent {

public:

	bool isSelected = false;
	bool isPressed = false;

	virtual void onSelect() = 0;
	virtual void onDeselect() = 0;
	virtual void onPress() = 0;
	virtual void onRelease() = 0;
	virtual void onOtherPressed() {};

	void onMouseClickEvent(MouseClickEvent& e) {

		float mousePosX = ((InputManager::localMouseState.position.x / (float)Window::width) * 2.f - 1.f) * ((float)Window::width/ (float)Window::height);
		float mousePosY = (InputManager::localMouseState.position.y / (float)Window::height) * 2.f - 1.f;

		std::cout << mousePosX << ", "
			<< mousePosY << std::endl;
		if (isSelected) {
			isPressed = true;
			onPress();
		}
		else {
			onOtherPressed();
		}
	}
	void onMouseReleaseEvent(MouseReleaseEvent& e) {
		if (isPressed) {
			isPressed = false;
			onRelease();
		}
	}

	void onMouseMovementEvent(MouseMovementEvent& e) {
		glm::mat3x3 mat = getGlobalMatrix();

		float mousePosX = ((InputManager::localMouseState.position.x / (float)Window::width) * 2.f - 1.f) * ((float)Window::width / (float)Window::height);
		float mousePosY = (InputManager::localMouseState.position.y / (float)Window::height) * 2.f - 1.f;
		
		glm::vec3 testPoint = glm::inverse(mat) * glm::vec3({ mousePosX, -mousePosY, 1 });

		if (testPoint.x < 1 && testPoint.x > 0 && testPoint.y < 1 && testPoint.y > 0) {
			if (!isSelected) {
				isSelected = true;
				onSelect();
			}
			return;
		}
		if (isSelected) {
			isSelected = false;
			onDeselect();
		}
	}

	UIButton(Actor& owner, Model* model, UIComponent* parent) : UIComponent(owner, model, parent) {
		Message<MouseClickEvent>::Connect(&UIButton::onMouseClickEvent, this);
		Message<MouseReleaseEvent>::Connect(&UIButton::onMouseReleaseEvent, this);
		Message<MouseMovementEvent>::Connect(&UIButton::onMouseMovementEvent, this);
	}

	~UIButton() {
		Message<MouseClickEvent>::Disconnect(&UIButton::onMouseClickEvent, this);
		Message<MouseReleaseEvent>::Disconnect(&UIButton::onMouseReleaseEvent, this);
		Message<MouseMovementEvent>::Disconnect(&UIButton::onMouseMovementEvent, this);
	}
};