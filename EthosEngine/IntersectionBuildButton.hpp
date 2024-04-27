#pragma once
#include "UIButton.hpp"
#include "TextManager.hpp"

class IntersectionBuildButton : public UIButton {
	glm::vec4 unselectedColor = { 0.6f, 0.6f, 0.6f, 1.f };
	glm::vec4 selectedColor = { 1.0f, 1.0f, 1.0f, 1.f };
	void (*onPressLambda)(Actor& owner) = nullptr;
	void (*onReleaseLambda)(Actor& owner) = nullptr;

	TextComponent* label = nullptr;

public:
	IntersectionBuildButton(Actor& owner, Model* model, UIComponent* parent,
		float width, float height, std::string labelText, void (*onPressLambda)(Actor& owner)) :
		UIButton(owner, model, parent), onPressLambda(onPressLambda) {

		this->color = unselectedColor;
		this->width = 1.f;
		this->height = 1.f;
		this->transform.scale(glm::vec2(width, height));

		label = new TextComponent(owner, this, labelText);
		label->scalingMode = UIComponent::ScalingMode::UniformVertical;
		label->vertAlign = UIComponent::Alignment::Center;
		label->horzAlign = UIComponent::Alignment::Center;
		label->transform.setPosition({ 0.5, 0.5 });
	}

	~IntersectionBuildButton() {
		delete label;
	}

	void onSelect() override {
		color = selectedColor;
	};

	void onDeselect() override {
		color = unselectedColor;
	};

	void onPress() override {
		if (onPressLambda != nullptr)
			onPressLambda(owner);
	}

	void onRelease() override {
		if (onReleaseLambda != nullptr)
			onReleaseLambda(owner);
	}
};