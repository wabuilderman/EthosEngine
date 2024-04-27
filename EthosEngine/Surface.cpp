#include "Surface.hpp"

#include "Engine.hpp"
#include "WindowWrapper.hpp"

#include "engineUtil.hpp"

#include "Device.hpp"
#include "SyncManager.hpp"

#include <stdexcept>

VkSurfaceKHR Surface::surface = VK_NULL_HANDLE;

void Surface::drawFrame() {
	device->drawFrame(); // swapchain->drawFrame(syncManager, device->graphicsQueue, device->presentQueue);
}

Surface::Surface() {
	if (glfwCreateWindowSurface(*Engine::instance, *Engine::window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}

	DebugManager::message("Surface Created");
	device = new Device();
}

Surface::~Surface() {
	delete device;

	vkDestroySurfaceKHR(*Engine::instance, surface, nullptr);

	DebugManager::message("Surface Destroyed");
}