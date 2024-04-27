#pragma once
#include <vector>
class Actor;

typedef size_t ActorID;

class ActorManager {
	static std::vector<Actor*> actors;
	static std::vector<ActorID> actorsAwaitingDeletion;

public:
	static ActorID registerActor(Actor* actor);
	static void deregisterActor(ActorID id);
	static void scheduleActorDestruction(ActorID id);
	static Actor* getActorByID(ActorID id);
	static void cleanScheduledActors();
};