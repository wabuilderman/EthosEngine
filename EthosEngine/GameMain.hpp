#pragma once
#include "MyControls.hpp"
#include "MyMenuScene.hpp"

class Game {
	bool shouldClose = false;
	Scene* curScene = nullptr;

	void handleEsacpe(ExitProgram& e) {
		shouldClose = true;
	}

	void start();
	void end();
};