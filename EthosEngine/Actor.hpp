#pragma once

#include "RenderComponent.hpp"
#include "PhysicsComponent.hpp"
#include "TransformComponent.hpp"
#include "Model.hpp"
#include "ActorUpdateData.hpp"
#include <vector>
#include "ActorManager.hpp"

class Actor {
public:
	Actor* parent = nullptr;
	std::vector<Actor*> children;
	ActorID actorID;
	
	std::vector<RenderComponent*> renderComponents;
	TransformComponent* transformComponent = nullptr;
	PhysicsComponent* physicsComponent = nullptr;
	
	virtual const char* getClassName() const {
		return "Unknown Actor";
	};

	void addChild(Actor * child) {
		child->parent = this;
		children.push_back(child);
	}

	void addRenderComponent(Model* model = nullptr) {
		renderComponents.push_back(new RenderComponent(*this, model));
	}
	void removeRenderComponents() {
		for (RenderComponent* renderComponent : renderComponents) {
			if (renderComponent != nullptr) {
				delete renderComponent;
			}
		}
		renderComponents.clear();
	}

	void addTransformComponent() {
		transformComponent = new TransformComponent(*this);
	}
	void removeTransformComponent() {
		if (transformComponent != nullptr) {
			delete transformComponent;
			transformComponent = nullptr;
		}
	}

	void addPhysicsComponent(Actor* marker = nullptr) {
		physicsComponent = new PhysicsComponent(*this, marker);
	}
	void removePhysicsComponent() {
		if (physicsComponent != nullptr) {
			delete physicsComponent;
			physicsComponent = nullptr;
		}
	}

	void teleport(glm::vec3 location) {
		if (physicsComponent) {
			physicsComponent->teleport(location);
		} else if (transformComponent) {
			transformComponent->accessLocalTransform().setPosition(location);
		}
	}

	void applyUpdateData(const ActorUpdateData& data) {
		if (transformComponent) {
			transformComponent->accessLocalTransform().setPosition(data.translation);
			transformComponent->accessLocalTransform().setRotation(data.rotation);
			transformComponent->accessLocalTransform().setScale(data.scale);
		}

		if (physicsComponent) {
			physicsComponent->physicsBody.angularVelocity = data.angularVelocity;
			physicsComponent->physicsBody.velocity = data.velocity;
		}

		applyUpdateMetaData(data.metaData);
	}

	virtual void applyUpdateMetaData(const void* metadata) {}

	Actor(Actor* parent = nullptr) : parent(parent), actorID(ActorManager::registerActor(this)) {}
	virtual ~Actor() {
		for (Actor* child : children) {
			if (child) delete child;
		}

		if (transformComponent != nullptr) {
			delete transformComponent;
		}

		for (RenderComponent* renderComponent : renderComponents) {
			if (renderComponent != nullptr) {
				delete renderComponent;
			}
		}

		if (physicsComponent != nullptr) {
			delete physicsComponent;
		}
		ActorManager::deregisterActor(actorID);
	};
};