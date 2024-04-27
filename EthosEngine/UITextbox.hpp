#pragma once
#include "UIButton.hpp"
#include "TextManager.hpp"
#include "UpdateEvent.hpp"

class UITextbox : public UIButton {
public:
	static Model* getModel();

	UIComponent* textBackground = nullptr;
	UIComponent* cursor = nullptr;
	TextComponent* textComp = nullptr;
	std::string enteredText;
	std::string placeholderText;
	unsigned cursorIndex = 0;
	float blinkRate = 0.5f;
	float blinkAcc = 0.f;
	bool isActive = false;
	static constexpr glm::vec4 bgColor = { 0.5f, 0.5f, 0.5f, 1.f };
	static constexpr glm::vec4 borderColor = { 0.7f, 0.7f, 0.7f, 1.0f };
	static constexpr glm::vec4 selectColor = { 1.f, 1.f, 1.f, 1.0f };

	UITextbox(Actor& owner, UIComponent* parent, float width, float height);
	~UITextbox();

	void manageBlink(UpdateEvent& e);
	void updateCursorPos();
	void displayText();
	void onKeyHold(KeyHold& e);
	void onKeyPress(KeyPress& e);
	void onTextInput(TextInputEvent& e);
	void onPasteText(PasteEvent& e);

	void disengage();
	void engage();

	void virtual onEngage() = 0;
	void virtual onDisengage() = 0;

	void onPress() override;

	void onRelease() override {}
	void onSelect() override {
		color = selectColor;
	}
	void onDeselect() override {
		color = borderColor;
	}

	void onOtherPressed() override {
		if (isActive) {
			disengage();
		}
	}
};