#pragma once
#include "Component.hpp"
#include "Transform.hpp"

class TransformComponent : public Component {
	Transform localTransform;
	Transform globalTransform;

public:
	bool isDirty = true;

	TransformComponent(Actor& owner);
	virtual ~TransformComponent() {};

	void dirty();
	void clean();

	const Transform& getLocalTransform() const;
	Transform& accessLocalTransform();

	const Transform& getGlobalTransform() const;
};