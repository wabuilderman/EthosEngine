#pragma once
#include <vector>
#include "Actor.hpp"

class Scene {
public:
	std::vector<Actor*> actors;
	~Scene() {
		for (Actor* actor : actors) {
			delete actor;
		}
	}
};
