#pragma once
#include "Actor.hpp"
#include "RectMesh.hpp"

#include "UIComponent.hpp"
#include "RenderComponentManager.hpp"
#include <sstream>
#include <iomanip>

#include "TextManager.hpp"
#include "InputManager.hpp"
#include "UpdateEvent.hpp"
#include "UIButton.hpp"

#include "NetworkManager.hpp"
#include "MainMenuButton.hpp"

#include "IPentryTextbox.hpp"
#include "MyControls.hpp"
class PortEntryTextbox : public UITextbox {
public:
	int portNumber;
	PortEntryTextbox(Actor& owner, UIComponent* parent, float width, float height)
		: UITextbox(owner, parent, width, height) {
		textComp->transform.setScale({ 0.5f, 0.5f });
		placeholderText = "";
		enteredText = "8001";
		cursorIndex = 4;
		displayText();
		cursor->isVisible = false;
	}
	void onEngage() {
		MyControls::getControls().playerMovement.lock = true;
	}
	void onDisengage() {
		MyControls::getControls().playerMovement.lock = false;
		portNumber = atoi(enteredText.c_str());
	}
};

class MyMainMenuUI : public Actor {
public:
	IPentryTextbox* ipEntryBox;
	PortEntryTextbox* portEntryBox;

	Mesh* buttonMesh = nullptr;
	Model* buttonModel = nullptr;
	UIButton* hostButton = nullptr;
	UIButton* joinButton = nullptr;
	TextComponent* debugInfo = nullptr;

	void update(UpdateEvent& e) {
		std::stringstream stream;

		float fps = 0.f;
		if (e.dt != 0) {
			fps = 1.f / e.dt;
		}
		if (fps > 9999) { fps = 9999; }
		 
		stream << "dt: " << std::fixed << std::setw(8) << std::setprecision(2) << (e.dt * 1000.f) << "ms"
			;//<< " (" << std::setw(7) << std::setprecision(2) << fps << ")";
		std::string s = stream.str();
		debugInfo->setText(s);
	}

	MyMainMenuUI(Actor* parent = nullptr) : Actor(parent) {

		ipEntryBox = new IPentryTextbox(*this, nullptr, 0.5f, 0.2f);
		ipEntryBox->transform.setPosition(glm::vec2(0, -0.25f));
		portEntryBox = new PortEntryTextbox(*this, nullptr, 0.5, 0.2f);
		portEntryBox->transform.setPosition({ 0, 0.25f });

		buttonMesh = new RectMesh();
		buttonModel = new Model(buttonMesh, nullptr, nullptr);

		hostButton = new MainMenuButton(*this, buttonModel, nullptr, 0.8f, 0.1f, "Host", [](Actor& owner){
			MyMainMenuUI* menu = (MyMainMenuUI*)(&owner);
			NetworkManager::hostGame(menu->portEntryBox->portNumber);
		});

		joinButton = new MainMenuButton(*this, buttonModel, nullptr, 0.5f, 0.1f, "Join", [](Actor& owner){
			MyMainMenuUI* menu = (MyMainMenuUI*)(&owner);
			NetworkManager::joinGame(menu->ipEntryBox->address);
		});
		joinButton->transform.setPosition(glm::vec2(0.f, -0.5f));
		

		debugInfo = new TextComponent(*this);
		debugInfo->setText("dt: ");
		debugInfo->transform.scale(0.1f);
		debugInfo->transform.setPosition(glm::vec2(0, 0.5));
		debugInfo->vertAlign = UIComponent::Alignment::Center;
		debugInfo->horzAlign = UIComponent::Alignment::Center;
		transformComponent = new TransformComponent(*this);

		Message<UpdateEvent>::Connect(&MyMainMenuUI::update, this);
	}

	~MyMainMenuUI() {
		delete (MainMenuButton*) hostButton;
		delete (MainMenuButton*) joinButton;
		delete ipEntryBox;
		delete portEntryBox;
		delete buttonMesh;
		delete buttonModel;
		delete transformComponent;
	}
};