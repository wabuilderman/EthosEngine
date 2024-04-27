#include "IPentryTextbox.hpp"
#include "MyControls.hpp"

IPentryTextbox::IPentryTextbox(Actor& owner, UIComponent* parent, float width, float height)
	: UITextbox(owner, parent, width, height) {
	textComp->transform.setScale({ 0.5f, 0.5f });
	placeholderText = "[0::0]:0";
	displayText();
	cursor->isVisible = false;
}

void IPentryTextbox::onEngage() {
	MyControls::getControls().playerMovement.lock = true;
}

void IPentryTextbox::onDisengage() {
	MyControls::getControls().playerMovement.lock = false;
	address.Clear();
	address.ParseString(enteredText.c_str());

	char buf[80];
	address.ToString(&buf[0], 80, true);
	std::cout << buf << std::endl;
}