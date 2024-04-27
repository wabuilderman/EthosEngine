#include "SyncManager.hpp"

#include "engineUtil.hpp"

#include "Swapchain.hpp"

std::vector<VkSemaphore> SyncManager::imageAvailableSemaphores;
std::vector<VkSemaphore> SyncManager::renderFinishedSemaphores;
std::vector<VkFence> SyncManager::inFlightFences;
std::vector<VkFence> SyncManager::imagesInFlight;

const int SyncManager::MAX_FRAMES_IN_FLIGHT = 2;
size_t SyncManager::currentFrame = 0;

void SyncManager::incrementFrame() {
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void SyncManager::checkFrame(uint32_t imageIndex) {
	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(Device::logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}
	// Mark the image as now being in use by this frame
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];
}

SyncManager::SyncManager(Swapchain& swapchain) {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(swapchain.frames.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(Device::logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(Device::logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(Device::logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}

	DebugManager::message("SyncManager Created");
}

SyncManager::~SyncManager() {
	vkDeviceWaitIdle(Device::logicalDevice);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(Device::logicalDevice, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(Device::logicalDevice, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(Device::logicalDevice, inFlightFences[i], nullptr);
	}

	DebugManager::message("SyncManager Destroyed");
}