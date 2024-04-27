#include "PhysicsManager.hpp"

PhysicsManager& PhysicsManager::getPhysicsManager() {
	static PhysicsManager physicsManager;
	return physicsManager;
}

void PhysicsManager::activate() {
	PhysicsManager& pm = getPhysicsManager();
	Message<UpdateEvent>::Connect(&PhysicsManager::update, &pm);
	pm.active = true;
}

void PhysicsManager::deactivate() {
	PhysicsManager& pm = getPhysicsManager();
	Message<UpdateEvent>::Disconnect(&PhysicsManager::update, &pm);
	pm.active = false;
}

void PhysicsManager::setPause(bool shouldPause) {
	PhysicsManager& pm = getPhysicsManager();
	pm.isPaused = shouldPause;
}

bool PhysicsManager::getIsPaused() {
	PhysicsManager& pm = getPhysicsManager();
	return pm.isPaused;
}

void PhysicsManager::addPhysicsComponent(PhysicsComponent* physicsComponent) {
	if (!getPhysicsManager().active) activate();
	getPhysicsManager().physicsComponents.push_back(physicsComponent);
}

void PhysicsManager::removePhysicsComponent(PhysicsComponent* physicsComponent) {
	PhysicsManager& pm = getPhysicsManager();
	for (unsigned i = 0; i < pm.physicsComponents.size(); ++i) {
		if (pm.physicsComponents[i] == physicsComponent) {
			pm.physicsComponents[pm.physicsComponents.size() - 1] = pm.physicsComponents[i];
			pm.physicsComponents.pop_back();
		}
	}

	if (pm.physicsComponents.size() == 0) {
		deactivate();
	}
}

void PhysicsManager::simulate(float dt) {
	PhysicsManager& pm = getPhysicsManager();
	// Apply forces
	for (PhysicsComponent* physicsComponent : pm.physicsComponents) {
		physicsComponent->physicsBody.calculateForces(dt);
	}

	// Update target transforms
	for (PhysicsComponent* physicsComponent : pm.physicsComponents) {
		physicsComponent->physicsBody.calculateTargetTransform(dt);
	}

	// Perform collision-checks between all physics components
	for (int i = 0; i < pm.physicsComponents.size() - 1; ++i) {
		for (int j = i + 1; j < pm.physicsComponents.size(); ++j) {
			pm.physicsComponents[i]->physicsBody.doIntersect(pm.physicsComponents[j]->physicsBody, dt);
		}
	}

	// Apply all target transforms
	for (PhysicsComponent* physicsComponent : pm.physicsComponents) {
		physicsComponent->physicsBody.currentTransform = physicsComponent->physicsBody.targetTransform;
	}

	// Update transform components
	for (PhysicsComponent* physicsComponent : pm.physicsComponents) {
		physicsComponent->updateTransform();
	}
}

void PhysicsManager::update(UpdateEvent& e) {
	static float timeElapsed = 0.f;
	timeElapsed += e.dt;
	if (timeElapsed >= physicsTimestep && !isPaused) {
		timeElapsed -= physicsTimestep;
		timeElapsed = fminf(timeElapsed, physicsTimestep);
		simulate(physicsTimestep);
	}
}