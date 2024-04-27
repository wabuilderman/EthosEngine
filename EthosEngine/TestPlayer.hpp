#pragma once
#include "Actor.hpp"
#include "CubeMesh.hpp"
#include "MyControls.hpp"
#include "NetworkManager.hpp"

class TestPlayer : Actor {
public:
	Model* boxModel = nullptr;
	Material myMat = Material();
	unsigned playerID = 0;

	void handlePlayerMovement(PlayerMovement& playerMovment) {
		if (playerID != NetworkManager::getPlayerID()) {
			return;
		}

		glm::vec3 movement
			= playerMovment.forwardAxis * Vector::forward
			+ playerMovment.verticalAxis * Vector::up
			+ playerMovment.sidewaysAxis * Vector::left;
		transformComponent->accessLocalTransform().moveRelative(playerMovment.dt * movement);

		ActorUpdateData updateData;
		updateData.actorID = actorID;
		updateData.translation = transformComponent->getLocalTransform().T;
		updateData.rotation = transformComponent->getLocalTransform().R;
		updateData.scale = transformComponent->getLocalTransform().S;

		NetworkManager::updateActor(updateData);
	}

	TestPlayer(unsigned playerID) {
		this->playerID = playerID;
		Mesh* boxMesh = CubeMesh::get();
		boxModel = new Model(boxMesh, nullptr, nullptr);
		addRenderComponent(boxModel);
		transformComponent = new TransformComponent(*this);
		float x = ((rand() % 1001) / 1000.f) * 2.f - 1.f;
		float y = ((rand() % 1001) / 1000.f) * 2.f - 1.f;
		transformComponent->accessLocalTransform().setPosition(
			2.f * Vector::forward
			+ x * Vector::up
			+ y * Vector::left);
		Message<PlayerMovement>::Connect(&TestPlayer::handlePlayerMovement, this);
	}

	~TestPlayer() {
		Message<PlayerMovement>::Disconnect(&TestPlayer::handlePlayerMovement, this);
		delete boxModel;
	}
};