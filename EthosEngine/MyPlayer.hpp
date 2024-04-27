#pragma once
#include "Actor.hpp"
#include "MyControls.hpp"
#include "MyCamera.hpp"

class MyPlayer : public Actor {
public:
	PhysicsShape* physicsShape = nullptr;
	MyCamera* camera = nullptr;

	void handleCameraControl(CameraControl& cameraControls) {
		//physicsComponent->physicsBody.angularVelocity 
		//transformComponent->accessLocalTransform().rotateAbsolute(glm::angleAxis(cameraControls.dt * glm::radians(90.0f) * -(float)cameraControls.yawAxis, Vector::up));
	}

	void handlePlayerMovement(PlayerMovement& playerMovment) {
		glm::vec3 movement
			= playerMovment.forwardAxis * Vector::forward
			+ playerMovment.verticalAxis * Vector::up
			+ playerMovment.sidewaysAxis * Vector::left;
		movement = glm::rotate(
			glm::angleAxis(camera->yaw * glm::radians(360.0f), Vector::up), glm::vec4(movement, 0));

		//physicsComponent->physicsBody.setLinearVelocity(movement);
		transformComponent->accessLocalTransform().moveRelative(playerMovment.dt * movement);
	}

	MyPlayer() {
		addTransformComponent();
		//physicsShape = new SpherePhysicsShape(0.5f);
		//physicsComponent = new PhysicsComponent(*this);
		//physicsComponent->physicsBody.shapes.push_back(physicsShape);
		//physicsComponent->physicsBody.noCollision = true;

		camera = new MyCamera(this);
		children.push_back(camera);

		Message<CameraControl>::Connect(&MyPlayer::handleCameraControl, this);
		Message<PlayerMovement>::Connect(&MyPlayer::handlePlayerMovement, this);
	}

	~MyPlayer() {
		Message<CameraControl>::Disconnect(&MyPlayer::handleCameraControl, this);
		Message<PlayerMovement>::Disconnect(&MyPlayer::handlePlayerMovement, this);
	}
};