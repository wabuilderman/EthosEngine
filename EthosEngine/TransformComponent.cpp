#include "TransformComponent.hpp"
#include "Actor.hpp"
#include "Transform.hpp"

TransformComponent::TransformComponent(Actor& owner) : Component(owner) {
	dirty();
}

const Transform& TransformComponent::getLocalTransform() const {
	return localTransform;
}

Transform& TransformComponent::accessLocalTransform() {
	dirty();
	return localTransform;
}

void TransformComponent::dirty() {
	isDirty = true;

	// Dirty all children of this object
	for (Actor* child : owner.children) {
		child->transformComponent->dirty();
	}
}

void TransformComponent::clean() {
	// If dirty, calculate matrices to next clean ancestor
	if (isDirty) {
		if (owner.parent) {
			globalTransform = owner.parent->transformComponent->getGlobalTransform().getMatrix() * localTransform.getMatrix();
		}
		else {
			globalTransform = localTransform.getMatrix();
		}
		isDirty = false;
	}
}

const Transform& TransformComponent::getGlobalTransform() const {
	const_cast<TransformComponent*>(this)->clean();
	return globalTransform;
}