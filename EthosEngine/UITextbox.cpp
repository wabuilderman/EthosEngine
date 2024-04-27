#include "UITextbox.hpp"
#include "RenderComponentManager.hpp"
#include "RectMesh.hpp"

Model* UITextbox::getModel() {
	static Model model(RectMesh::get(), nullptr, nullptr);
	return &model;
}

void UITextbox::engage() {
	Message<KeyPress>::Connect(&UITextbox::onKeyPress, this);
	Message<KeyHold>::Connect(&UITextbox::onKeyHold, this);
	Message<TextInputEvent>::Connect(&UITextbox::onTextInput, this);
	Message<UpdateEvent>::Connect(&UITextbox::manageBlink, this);
	Message<PasteEvent>::Connect(&UITextbox::onPasteText, this);
	cursor->isVisible = true;
	isActive = true;
	displayText();
	onEngage();
}

void UITextbox::disengage() {
	Message<KeyPress>::Disconnect(&UITextbox::onKeyPress, this);
	Message<KeyHold>::Disconnect(&UITextbox::onKeyHold, this);
	Message<TextInputEvent>::Disconnect(&UITextbox::onTextInput, this);
	Message<UpdateEvent>::Disconnect(&UITextbox::manageBlink, this);
	Message<PasteEvent>::Disconnect(&UITextbox::onPasteText, this);
	isActive = false;
	displayText();
	cursor->isVisible = false;
	onDisengage();
}
UITextbox::UITextbox(Actor& owner, UIComponent* parent, float width, float height) : UIButton(owner, UITextbox::getModel(), parent) {
	textBackground = new UIComponent(owner, UITextbox::getModel(), this);
	this->width = 1.f;
	this->height = 1.f;
	this->horzAlign = UIComponent::Alignment::Center;
	this->vertAlign = UIComponent::Alignment::Center;
	this->transform.setScale({ width, height });
	color = borderColor;

	float subWidth = (width - 0.05f) / width;
	float subHeight = (height - 0.05f) / height;

	textBackground->width = 1.f;
	textBackground->height = 1.f;
	textBackground->transform.setScale({ subWidth, subHeight });
	textBackground->horzAlign = UIComponent::Alignment::Center;
	textBackground->vertAlign = UIComponent::Alignment::Center;
	textBackground->color = bgColor;
	textBackground->transform.setPosition({ 0.5f, 0.5f });

	textComp = new TextComponent(owner, this);
	textComp->scalingMode = ScalingMode::UniformVertical;
	textComp->horzAlign = UIComponent::Alignment::Center;
	textComp->vertAlign = UIComponent::Alignment::Center;
	textComp->transform.setPosition({ 0.5f, 0.5f });

	cursor = new UIComponent(owner, UITextbox::getModel(), textComp);
	cursor->width = 1.f;
	cursor->height = 1.f;
	cursor->scalingMode = ScalingMode::UniformVertical;
	cursor->vertAlign = UIComponent::Alignment::Center;
	cursor->transform.setPosition({ 0.5f, 0.5f });
	cursor->isVisible = false;
	cursor->transform.setScale({ 0.07f, 1.f });
}

UITextbox::~UITextbox() {
	delete cursor;
	delete textBackground;
}

void UITextbox::manageBlink(UpdateEvent& e) {
	if ((blinkAcc += e.dt) > blinkRate) {
		blinkAcc = 0.f;
		cursor->isVisible = !cursor->isVisible;
	}
}

void UITextbox::updateCursorPos() {
	blinkAcc -= blinkRate / 2;
	blinkAcc = (blinkAcc < 0.f ? 0 : blinkAcc);
	cursor->isVisible = true;
	cursor->transform.setPosition({ textComp->getCharOffset(cursorIndex), 0.5f });
}

void UITextbox::displayText() {
	if (isActive || enteredText.length() > 0) {
		textComp->setText(enteredText);
	}
	else {
		textComp->setText(placeholderText);
	}
	updateCursorPos();
}

void UITextbox::onKeyHold(KeyHold& e) {
	if (e.keycode == GLFW_KEY_BACKSPACE) {
		if (enteredText.length() > 0) {
			if (enteredText.length() == 1) {
				enteredText = "";
			}
			else {
				enteredText.erase(cursorIndex - 1);
			}
			cursorIndex--;
			displayText();
		}
	}
	else if (e.keycode == GLFW_KEY_LEFT) {
		if (cursorIndex > 0) {
			cursorIndex--;
			updateCursorPos();
		}
	}
	else if (e.keycode == GLFW_KEY_RIGHT) {
		if (cursorIndex < enteredText.length()) {
			cursorIndex++;
			updateCursorPos();
		}
	}
}

void UITextbox::onKeyPress(KeyPress& e) {
	if (e.keycode == GLFW_KEY_BACKSPACE) {
		if (enteredText.length() > 0) {
			if (enteredText.length() == 1) {
				enteredText = "";
			}
			else {
				enteredText.erase(cursorIndex - 1);
			}
			cursorIndex--;
			displayText();
		}
	}
	else if (e.keycode == GLFW_KEY_ENTER) {
		disengage();
	}
	else if (e.keycode == GLFW_KEY_LEFT) {
		if (cursorIndex > 0) {
			cursorIndex--;
			updateCursorPos();
		}
	}
	else if (e.keycode == GLFW_KEY_RIGHT) {
		if (cursorIndex < enteredText.length()) {
			cursorIndex++;
			updateCursorPos();
		}
	}
}

void UITextbox::onTextInput(TextInputEvent& e) {
	char c;
	if (e.codepoint > 0xFFu) {
		c = (char)0xFFu;
	}
	else {
		c = (char)e.codepoint;
	}

	enteredText.insert(cursorIndex++, 1, c);
	displayText();
}

void UITextbox::onPasteText(PasteEvent& e) {
	assert(e.text != nullptr);
	enteredText.insert(cursorIndex, e.text);
	for (int i = 0; e.text[i] != '\0'; ++i) {
		cursorIndex++;
	}
	displayText();
};

void UITextbox::onPress() {
	engage();
}