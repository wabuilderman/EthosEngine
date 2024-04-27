#include "PhysicsBody.hpp"
#include "Actor.hpp"
#include "PhysicsManager.hpp"

PhysicsBody::PhysicsBody(Actor& owner, Actor* marker) : Component(owner) {
	this->marker = marker;
	teleport(owner.transformComponent->getGlobalTransform().getPosition());
}

PhysicsBody::~PhysicsBody() {

}

bool PhysicsBody::doShapeIntersect(PhysicsShape* shapeA, PhysicsShape* shapeB, PhysicsBody& other, float dt) {
	const float margin = 0.0001f;
	bool usedRaycastGJK = true;

	Collision collision(this, &other, shapeA, shapeB);

	// deltaPos is calculated as B's motion relative to A.
	glm::vec3 bRelPos = other.currentTransform.getPosition() - currentTransform.getPosition();

	if (glm::length2(other.velocity - velocity) < FLT_EPSILON * FLT_EPSILON && glm::length2(other.angularVelocity) < FLT_EPSILON * FLT_EPSILON && glm::length2(angularVelocity) < FLT_EPSILON * FLT_EPSILON) {
		return false;
	}

	if (collision.handleIntersect((velocity + other.velocity) * dt, usedRaycastGJK)) {

		targetTransform.setRotation(currentTransform.getRotation());
		other.targetTransform.setRotation(other.currentTransform.getRotation());

		// Assert collision normal is not NaN
		assert(collision.normal == collision.normal);

		// Handle collisions with static objects
		if (other.isStatic) {
			collision.normal = isStatic ? -collision.normal : collision.normal;


			if (usedRaycastGJK) {
				assert(collision.normal == collision.normal);       // NaN check
				assert(collision.normal != glm::zero<glm::vec3>()); // Zero check

				collision.normal = glm::normalize(collision.normal);

				glm::vec3 hs = targetTransform.getPosition() + collision.hitSpot - other.targetTransform.getPosition();
				glm::vec3 deltaPos = velocity * dt * (collision.time);
				// Calculate component of velocity parallel to collision normal
				glm::vec3 normalVel = glm::dot(collision.normal, velocity) * collision.normal;
				applyImpulse(-normalVel, hs);
				deltaPos -= collision.normal * margin;
				
				
				glm::quat rotationA = glm::slerp(this->currentTransform.getRotation(), this->targetTransform.getRotation(), collision.time);
				glm::quat rotationB = glm::slerp(other.currentTransform.getRotation(), other.targetTransform.getRotation(), collision.time);

				glm::vec3 supportA = rotationA * shapeA->support(-collision.normal * rotationA);
				glm::vec3 supportB = rotationB * shapeB->support(collision.normal * rotationB);

				glm::vec3 globalHitspot = collision.hitSpot;
				if (marker) {
					marker->teleport(globalHitspot);
				}

				

				std::cout
					<< "iter:" << collision.GJK_Iterations
					<< ", vel: " << velocity
					<< ", norm: " << collision.normal
					<< ", deltapos: " << deltaPos
					<< ", hs: " << globalHitspot
					<< ", cm: " << collision.hitSpot
					<< ", tg: " << targetTransform.getPosition()
					<< ", supA: " << supportA
					<< ", supB: " << supportB
					<< ", time: " <<collision.time
					<< std::endl;

				//std::cout << "targetPos: " << nonStatic.targetTransform.getPosition() << ", currentPos: " << nonStatic.currentTransform.getPosition() << ", finalPos: " << (nonStatic.currentTransform.getPosition() + deltaPos) << std::endl;
				targetTransform.setPosition(currentTransform.getPosition() + deltaPos);
				PhysicsManager::setPause(true);
				
			}
			else {
				glm::vec3 hs = collision.hitSpot - targetTransform.getPosition();
				float normalVelMag = glm::dot(collision.normal, velocity);
				glm::vec3 normalVel = normalVelMag * glm::normalize(collision.normal) * 1.001f;
				glm::vec3 tanVel = glm::dot(collision.normal, velocity) * glm::normalize(collision.normal);

				// Apply ejection-impulse
				applyImpulse(-normalVel, hs);

				// Apply surface friction
				float frictionCoefficent = 0.6f;

				// NOTE: this should use force, not velocity. That said; it works. :P
				float frictionMagnitude = fmaxf(fminf(normalVelMag * frictionCoefficent, 1.f), 0.f);
				//applyAngularImpulse(angularVelocity * -frictionMagnitude);
				//applyImpulse(velocity * -frictionMagnitude, hs);

				targetTransform.setRotation(currentTransform.R);
				targetTransform.addVectorRotation(angularVelocity, dt);

				if (glm::length2(normalVel) > FLT_EPSILON * FLT_EPSILON) {
					targetTransform.setPosition(
						targetTransform.getPosition() - (collision.normal * (collision.distance + margin))
					);
				}



				assert(targetTransform.getPosition() == targetTransform.getPosition());

				if (marker) {
					marker->teleport(hs);
				}
				

				std::cout
					<< "GJK: " << collision.GJK_Iterations
					<< ", EPA: " << collision.EPA_Iterations
					<< ", EPA_faces: " << collision.EPA_FaceCount
					<< ", EPA_verts: " << collision.EPA_VertexCount
					<< std::endl;

				std::cout
					<< "vel: " << velocity
					<< ", angVel: " << angularVelocity
					<< ", norm: " << collision.normal
					<< ", normVel: " << normalVel
					<< std::endl;

				std::cout
					<< ", hitspotLcl: " << hs
					<< ", hitspotGbl: " << collision.hitSpot
					<< ", dist: " << collision.distance
					<< std::endl;
			}
		}
		else {
			targetTransform.setPosition(currentTransform.getPosition());// +deltaPosA * collision.time - glm::normalize(deltaPosA) * margin);
			other.targetTransform.setPosition(other.currentTransform.getPosition()); //+ deltaPosB * collision.time - glm::normalize(deltaPosB) * margin);

			std::cout << "iter:" << collision.GJK_Iterations << ", norm: " << collision.normal << std::endl;

			// For now, completely inelastic
			glm::vec3 combinedMomentum = momentum + other.momentum;
			glm::vec3 finalVelocity = combinedMomentum / (mass + other.mass);

			setLinearVelocity(finalVelocity);
			other.setLinearVelocity(finalVelocity);
		}
		return true;
	}
	return false;

}

void PhysicsBody::applyPhysicsTransfrom() {
	owner.transformComponent->accessLocalTransform() = currentTransform.getMatrix();
}

void PhysicsBody::teleport(glm::vec3 position) {
	targetTransform.setPosition(position);
	currentTransform.setPosition(position);
}