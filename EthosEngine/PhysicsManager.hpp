#pragma once
#include "UpdateEvent.hpp"
#include "Message.hpp"
#include "Actor.hpp"

class PhysicsManager {
public:
	bool active = false;
	bool isPaused = false;
	std::vector<PhysicsComponent*> physicsComponents;

	static constexpr float physicsFramerate = 10.0f;
	static constexpr float physicsTimestep = (float)(1.0 / (double)physicsFramerate);

private:
	static PhysicsManager& getPhysicsManager();
	static void activate();
	static void deactivate();

public:

	static void addPhysicsComponent(PhysicsComponent* physicsComponent);
	static void removePhysicsComponent(PhysicsComponent* physicsComponent);
	static void simulate(float dt);
	static void setPause(bool shouldPause);
	static bool getIsPaused();
	void update(UpdateEvent& e);
};