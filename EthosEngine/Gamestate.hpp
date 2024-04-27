#pragma once
#include "ActorUpdateData.hpp"
#include "NetMessageTypes.hpp"
#include <map>
#include <array>

class Gamestate {
private:
	const NetMessageType sig = NetMessageType::Gamestate;

public:
	static constexpr unsigned maxActorUpdates = 10;
	unsigned updateCount = 0;
	std::array<ActorUpdateData, maxActorUpdates> actorUpdates;

	void addUpdate(const ActorUpdateData& data) {
		bool added = false;
		for (unsigned i = 0; i < updateCount; ++i) {
			if (actorUpdates[i].actorID == data.actorID) {
				actorUpdates[i] = data;
				added = true;
				break;
			}
		}
		if (!added) {
			actorUpdates[updateCount++] = data;
		}
	}

	void operator+=(const Gamestate* gs) {
		for (unsigned i = 0; i < gs->updateCount; ++i) {
			addUpdate(gs->actorUpdates[i]);
		}
	}
};