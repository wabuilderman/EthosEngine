#include "RenderComponent.hpp"
#include "Texture.hpp"
#include "RenderComponentManager.hpp"
#include "Actor.hpp"

RenderComponent::RenderComponent(Actor& owner, Model* model) : Component(owner), model(model) {
	if (model) {
		RenderComponentManager::addRenderComponent(this);
	}
	
}

void RenderComponent::setModel(Model* model) {
	if (this->model == model) {
		return;
	}

	if (this->model) {
		RenderComponentManager::removeRenderComponent(this);
	}

	this->model = model;
	RenderComponentManager::addRenderComponent(this);
}

RenderComponent::~RenderComponent() {
	RenderComponentManager::removeRenderComponent(this);
}