#pragma once
#include "Transform.hpp"

class PhysicsShape {
public:
	glm::mat3x3 interiaTensor = glm::identity<glm::mat3x3>(); // multiply by mass first
	// GJK support function - Returns point with largest dot-product with respect to the given vector, in global space
	// Perhaps require returning centroid of faces/edges which are normal to the vector?
	virtual glm::vec3 support(glm::vec3 dir) const = 0;
	virtual glm::vec3 supportNormal(glm::vec3 dir) const = 0;
};

class SpherePhysicsShape : public PhysicsShape {
public:
	float radius;
	SpherePhysicsShape(float radius = 1.0f);

	// GJK suppport function
	glm::vec3 support(glm::vec3 dir) const override;
	glm::vec3 supportNormal(glm::vec3 dir) const override;
};

class BoxPhysicsShape : public PhysicsShape {
public:
	glm::vec3 dim;
	BoxPhysicsShape(glm::vec3 dim = { 1.f, 1.f, 1.f });

	// GJK suppport function
	glm::vec3 support(glm::vec3 dir) const override;
	glm::vec3 supportNormal(glm::vec3 dir) const override;
};