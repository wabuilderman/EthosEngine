#pragma once
#include "Component.hpp"
#include "PhysicsShape.hpp"
#include "Transform.hpp"
#include "Collision.hpp"

#include <vector>
#include <iostream>

static std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
	return os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
}

class PhysicsBody : public Component {
public:
	std::vector<PhysicsShape*> shapes;
	Actor* marker;

	// Seperate transform used to enable asynchronus computation
	Transform currentTransform;
	Transform targetTransform;

	float mass = 1.0f; // Mass in kg

	// Acceleration due to gravity, NOT the gravitational constant
	static constexpr float gravityConstant = 9.81f;

	float linearVelocityDamping = 0.01f;
	float angularVelocityDamping = 0.01f;

	float coefficientOfRestitution = 0.5f;
	glm::vec3 offsetToCenterOfMass = {};
	
	glm::vec3 velocity = { 0, 0, 0 };
	glm::vec3 angularVelocity = { 0, 0, 0 };

	//glm::vec3 acceleration = { 0, 0, 0 };
	//glm::vec3 angularAcceleration = { 0, 0, 0 };

	glm::vec3 momentum = { 0, 0, 0 };
	glm::vec3 angularMomentum = { 0, 0, 0 };

	float drag = 0.1f;
	bool isAwake = false;
	bool isStatic = false;
	bool noCollision = false;
	bool enableGravity = false;

	PhysicsBody(Actor& owner, Actor*);
	~PhysicsBody();

	void applyPhysicsTransfrom();

	void calculateForces(float dt) {
		// Apply gravity
		if (enableGravity) {
			velocity += Vector::down * gravityConstant * dt;
		}

		// Apply damping
		velocity = Vector::damp(velocity, linearVelocityDamping * dt);
		angularVelocity = Vector::damp(angularVelocity, angularVelocityDamping * dt);
	}

	void calculateTargetTransform(float dt) {
		targetTransform = currentTransform;

		if (glm::length2(velocity) > FLT_EPSILON * FLT_EPSILON) {
			targetTransform.moveAbsolute(velocity * dt);
		}

		if (glm::length2(angularVelocity) > FLT_EPSILON * FLT_EPSILON) {
			targetTransform.addVectorRotation(angularVelocity, dt);
		}
	}

	void teleport(glm::vec3 position);

	void setEnableGravity(bool enable) {
		enableGravity = enable;
	}

	void applyImpulse(glm::vec3 impulse) {
		isAwake = true;
		velocity += impulse;
		momentum = velocity / mass;
	}

	void applyImpulse(glm::vec3 v, glm::vec3 r) {
		isAwake = true;
		// w = r x v / (r*r)
		// H_g = int_m ( r x v ) dm

		glm::vec3 w = glm::cross(r, v) * (1 / glm::dot(r, r));
		this->angularVelocity += w;
		this->velocity += glm::dot(v, r) * (1 / glm::dot(r, r)) * r;
	}

	void applyAngularImpulse(glm::vec3 w) {
		this->angularVelocity += w;
	}

	void setLinearVelocity(glm::vec3 velocity) {
		this->isAwake = true;
		this->velocity = velocity;
		this->momentum = velocity / this->mass;
	}

	void setAngularVelocity(glm::vec3 angularVelocity) {
		this->angularVelocity = angularVelocity;
	}

private:
	bool doShapeIntersect(PhysicsShape* shapeA, PhysicsShape* shapeB, PhysicsBody& other, float dt);

	bool doIntersectReordered(PhysicsBody& other, float dt) {
		bool hasIntersection = false;
		for (PhysicsShape* shapeA : shapes) {
			for (PhysicsShape* shapeB : other.shapes) {
				hasIntersection |= doShapeIntersect(shapeA, shapeB, other, dt);
			}
		}
		return hasIntersection;
	}

public:
	bool doIntersect(PhysicsBody& other, float dt) {
		// If collision should be ignored, return false
		if ( (noCollision || other.noCollision) || (isStatic && other.isStatic) ) {
			return false;
		}

		if (isStatic) {
			return other.doIntersectReordered(*this, dt);
		} else {
			return doIntersectReordered(other, dt);
		}
	}
};

