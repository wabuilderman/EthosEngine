#include "PhysicsComponent.hpp"
#include "Actor.hpp"
#include "PhysicsManager.hpp"
#include "Transform.hpp"

PhysicsComponent::PhysicsComponent(Actor& owner, Actor* marker) : Component(owner), physicsBody(owner, marker) {
	PhysicsManager::addPhysicsComponent(this);
}

void PhysicsComponent::teleport(const Vector& position) {
	physicsBody.teleport(position);
	owner.transformComponent->accessLocalTransform().setPosition(position);
}

void PhysicsComponent::updateTransform() {
	physicsBody.applyPhysicsTransfrom();
}

PhysicsComponent::~PhysicsComponent() {
	PhysicsManager::removePhysicsComponent(this);
}