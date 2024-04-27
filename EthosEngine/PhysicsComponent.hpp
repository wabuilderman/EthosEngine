#pragma once
#include "Component.hpp"
#include "Transform.hpp"
#include "PhysicsBody.hpp"
#include <vector>


class PhysicsComponent : public Component {
public:
	PhysicsBody physicsBody;

	PhysicsComponent(Actor& owner, Actor* marker = nullptr);
	void teleport(const Vector& position);
	void updateTransform();
	~PhysicsComponent();
};