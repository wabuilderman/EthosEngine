#pragma once

#include "Actor.hpp"
#include "InputManager.hpp"
#include "CameraManager.hpp"
#include "MyControls.hpp"

#include "JoystickUI.hpp"
#include "TestObject.hpp"

#include <algorithm>

class MyCamera : public Actor {
public:
	float pitch = 0.0f;
	float yaw = 0.0f;
	bool lockCamera = false;

	void handleCameraControls(CameraControl& cameraControls) {
		if(!lockCamera) {
			// Pitch ranges from -1 to 1, mapping to -90 to +90 degrees of rotation
			pitch = std::clamp(pitch + cameraControls.dt * (float)cameraControls.pitchAxis, -1.0f, 1.0f);

			// Yaw ranges from 0 to 1, mapping to 0 to 360 degrees of rotation
			yaw = fmod(yaw + cameraControls.dt * 0.2f * -(float)cameraControls.yawAxis, 1.0f);

			transformComponent->accessLocalTransform().setRotation(
				glm::angleAxis(yaw * glm::radians(360.0f), Vector::up) * glm::angleAxis(pitch * glm::radians(90.0f), Vector::left)
			);
		}
	}

	MyCamera(Actor * parent = nullptr) : Actor(parent) {
		addTransformComponent();

		Message<CameraControl>::Connect(&MyCamera::handleCameraControls, this);

		//children.push_back(new JoystickUI(this));
		CameraManager::setActiveCamera(this);
		
	}

	void update(float dt) {}

	~MyCamera() {
	}

};