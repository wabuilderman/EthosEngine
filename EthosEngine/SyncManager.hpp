#pragma once
#include "pch.hpp"

#include <vector>
#include "Swapchain.hpp"

class SyncManager {
public:

	static std::vector<VkSemaphore> imageAvailableSemaphores;
	static std::vector<VkSemaphore> renderFinishedSemaphores;
	static std::vector<VkFence> inFlightFences;
	static std::vector<VkFence> imagesInFlight;

	static const int MAX_FRAMES_IN_FLIGHT;

	static size_t currentFrame;

	SyncManager(Swapchain& swapchain);

	static VkSemaphore& currentImageAvailableSemaphore() {
		return imageAvailableSemaphores[currentFrame];
	}

	static VkSemaphore& currentRenderFinishedSemaphore() {
		return renderFinishedSemaphores[currentFrame];
	}

	static VkFence& currentInFlightFence() {
		return inFlightFences[currentFrame];
	}

	static VkFence& currentImageInFlight() {
		return imagesInFlight[currentFrame];
	}

	static void checkFrame(uint32_t imageIndex);

	static void incrementFrame();

	~SyncManager();
};