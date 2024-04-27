#pragma once

#include "Transform.hpp"
#include "Actor.hpp"
#include "Swapchain.hpp"

class CameraManager {
public:
	static Actor* cam;
	
	static void setActiveCamera(Actor* camera) {
		cam = camera;
	}

#pragma warning(push)
#pragma warning(disable:6011)

	static glm::mat4 calcViewMatrix() {
		assert(cam);
		assert(cam->transformComponent);

		return inverse(
			cam->transformComponent->getGlobalTransform().getMatrix()
			//* glm::mat4_cast(glm::quat(0.707f, 0, -0.707f, 0))
			* glm::mat4_cast(glm::quat(0.5f, 0.5f, -0.5f, -0.5f))
			//* glm::mat4_cast(glm::quat(0, 0, 0.707f, 0.707f))
		);
	}

	static glm::mat4 getViewMatrix() {
		assert(cam);
		assert(cam->transformComponent);

		// Keep inverse camera matrix around unless it needs to be rebuilt
		static glm::mat4 matrix = calcViewMatrix();

		if (cam->transformComponent->isDirty) {
			matrix = calcViewMatrix();
		}

		return matrix;
	}

	static glm::mat4 getProjectionMatrix() {
		return glm::perspective(glm::radians(60.0f), Swapchain::swapChainExtent.width / (float)Swapchain::swapChainExtent.height, 0.01f, 1000.0f);
	}

#pragma warning(pop)

	// For future 3d-support
	/*
	static constexpr float viewSeperation = 0.5f;
	static glm::mat4 getLeft3DViewMatrix() {
		assert(cam);
		assert(cam->transformComponent);
		glm::vec3 viewOffsetVec = { -viewSeperation, 0, 0 };
		glm::mat4 matrix = glm::translate(cam->transformComponent->getGlobalTransform().getMatrix() * glm::mat4_cast(glm::quat(0.0f, 0.0f, 1.0f, 0.0f) * glm::quat(0.707f, 0.0f, 0.707f, 0.0f)), viewOffsetVec);
		return inverse(matrix);
	}

	static glm::mat4 getRight3DViewMatrix() {
		assert(cam);
		assert(cam->transformComponent);
		glm::vec3 viewOffsetVec = { viewSeperation, 0, 0 };
		glm::mat4 matrix = glm::translate(cam->transformComponent->getGlobalTransform().getMatrix() * glm::mat4_cast(glm::quat(0.0f, 0.0f, 1.0f, 0.0f) * glm::quat(0.707f, 0.0f, 0.707f, 0.0f)), viewOffsetVec);
		return inverse(matrix);
	}
	*/
};