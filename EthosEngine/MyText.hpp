#pragma once
#include "Actor.hpp"
#include <map>

#include "TextManager.hpp"

class MyText : public Actor {
	Texture* texture = nullptr;
	Mesh* mesh = nullptr;
	Model* model = nullptr;

public:


	MyText() {
		FontManager::FontIdentity font = { FontManager::FontIdentity::Family::Arial, FontManager::FontIdentity::Quality::medium };

		std::string text = "Hello World";

		model = TextManager::generateText(text, font);

		addRenderComponent(model);
		transformComponent = new TransformComponent(*this);
		transformComponent->accessLocalTransform().setRotation(glm::angleAxis(glm::radians(90.0f), Vector::forward));
	}

	~MyText() {
		delete model;
	}
};