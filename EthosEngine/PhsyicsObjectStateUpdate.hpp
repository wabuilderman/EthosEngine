#pragma once
#include "pch.hpp"

struct PhysicObjectStateUpdate {
	unsigned int id; // Index into physicsmanager
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 linearVelocity;
	glm::vec3 angularVelocity;
};