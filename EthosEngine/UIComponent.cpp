#include "UIComponent.hpp"
#include "Texture.hpp"
#include "RenderComponentManager.hpp"
#include "Actor.hpp"

UIComponent::UIComponent(Actor& owner, Model* model, UIComponent* parent) : Component(owner), model(model), parent(parent) {
	if (model) {
		RenderComponentManager::addUIComponent(this);
	}

}

void UIComponent::setModel(Model* model) {
	if (this->model) {
		RenderComponentManager::removeUIComponent(this);
	}

	this->model = model;
	RenderComponentManager::addUIComponent(this);
}

UIComponent::~UIComponent() {
	RenderComponentManager::removeUIComponent(this);
}