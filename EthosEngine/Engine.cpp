#include "Engine.hpp"

#include "Instance.hpp"
#include "WindowWrapper.hpp"
#include "Device.hpp"

#include "InputManager.hpp"
#include "FontManager.hpp"
#include "SteamManager.hpp"
#include "UpdateEvent.hpp"

#include "PhysicsManager.hpp"

#include <chrono>

Window* Engine::window = nullptr;
Instance* Engine::instance = nullptr;

void Engine::init() {
	SteamManager::init();

	window = new Window();
	instance = new Instance();

	InputManager::init();
	FontManager::init();
}

void Engine::update() {
	// Record initial time, for valid 'dt' of first frame
	static auto prevTime = std::chrono::high_resolution_clock::now();

	SteamManager::update();
	window->pollEvents();

	// Render frame
	instance->drawFrame();

	// Record total time since last time this line of code was called
	auto currentTime = std::chrono::high_resolution_clock::now();

	UpdateEvent update = {
		.dt = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count()
	};

	InputManager::update(update.dt);
	ActorManager::cleanScheduledActors();

	Message<UpdateEvent>::Send(update);
	ActorManager::cleanScheduledActors();

	prevTime = currentTime;
}

void Engine::shutdown() {
	InputManager::shutdown();
	FontManager::shutdown();
	SteamManager::shutdown();

	delete instance;
	delete window;
}

void Engine::rest() {
	vkDeviceWaitIdle(Device::logicalDevice);
}

bool Engine::shouldClose() {
	return window->shouldClose();
}