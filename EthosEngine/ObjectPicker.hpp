#pragma once
#include "Actor.hpp"
#include "MyControls.hpp"
#include "UIComponent.hpp"
#include "TextManager.hpp"

class ObjectPicker : public Actor {
public:
	UIComponent* myUIComponent = nullptr;
	TextModel* model = nullptr;

	void update(UpdateEvent& e) {
		FontManager::FontIdentity font = { FontManager::FontIdentity::Family::Arial, FontManager::FontIdentity::Quality::medium };
		std::string text = std::to_string(e.dt);
		TextManager::editText(model, font, text, 0);
		myUIComponent->height = model->height;
		myUIComponent->width = model->width;
		RenderComponentManager::updateUIComponentVBO(myUIComponent);
	}

	ObjectPicker(Actor* parent = nullptr) : Actor(parent) {
		FontManager::FontIdentity font = { FontManager::FontIdentity::Family::Arial, FontManager::FontIdentity::Quality::medium };
		std::string text = "dt: This is a test string.";
		model = TextManager::generateText(text, font);
		myUIComponent = new UIComponent(*this, model, nullptr);
		myUIComponent->transform.scale(0.2f);
		transformComponent = new TransformComponent(*this);
		Message<UpdateEvent>::Connect(&ObjectPicker::update, this);
	};

	~ObjectPicker() {
		delete myUIComponent;
		delete model;
	};

};