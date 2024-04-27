#include "ActorManager.hpp"
#include "Actor.hpp"
std::vector<Actor*> ActorManager::actors;
std::vector<ActorID> ActorManager::actorsAwaitingDeletion;

ActorID ActorManager::registerActor(Actor* actor) {
	for (size_t i = 0; i < actors.size(); ++i) {
		if (actors[i] == nullptr) {
			actors[i] = actor;
			return (ActorID)i;
		}
	}

	actors.push_back(actor);
	return (ActorID)(actors.size() - 1ull);
}

void ActorManager::deregisterActor(ActorID id) {
	if (id != (ActorID)-1) {
		actors[(size_t)id]->actorID = (ActorID)-1;
		actors[(size_t)id] = nullptr;
	}
}

void ActorManager::scheduleActorDestruction(ActorID id) {
	actorsAwaitingDeletion.push_back(id);
}

Actor* ActorManager::getActorByID(ActorID id) {
	if (id == -1)
		return nullptr;
	else
		return actors[(size_t)id];
}

void ActorManager::cleanScheduledActors() {
	if (!actorsAwaitingDeletion.empty()) {
		for (ActorID id : actorsAwaitingDeletion) {
			Actor* actor = actors[(size_t)id];
			delete actor;
			actors[(size_t)id] = nullptr;
		}
		actorsAwaitingDeletion.clear();
	}
}