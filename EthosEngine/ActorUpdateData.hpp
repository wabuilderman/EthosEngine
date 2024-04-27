#pragma once
#include "ActorManager.hpp"
#include "NetMessageTypes.hpp"
#include "Transform.hpp"

class ActorUpdateData {
private:
	const NetMessageType sig = NetMessageType::ActorUpdate;
public:
	static constexpr unsigned maxMetaData = 64;

	ActorID actorID;

	// Transform Data
	glm::vec3 translation;
	glm::quat rotation;
	glm::vec3 scale;

	// Physics Data
	glm::vec3 velocity = { 0, 0, 0 };
	glm::vec3 angularVelocity = { 0, 0, 0 };

	// Meta Data
	float metaData[maxMetaData];

	void operator=(const ActorUpdateData& other) {
		memcpy(this, &other, sizeof(ActorUpdateData));
	}
};