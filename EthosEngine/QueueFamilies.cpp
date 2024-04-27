#include "QueueFamilies.hpp"

#include "engineUtil.hpp"

#include <vector>

bool QueueFamilies::QueueFamilyIndices::isComplete() {
	return graphicsFamily.has_value() && presentFamily.has_value();
}

QueueFamilies::QueueFamilyIndices QueueFamilies::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;

	// Query number of queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	// Create list of queue families
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// Find queue families that support each of the desired criteria
	for (int i = 0; i < queueFamilies.size(); ++i) {
		// Flag graphics support
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		// Query present support
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		// Flag present support
		if (presentSupport) {
			indices.presentFamily = i;
		}

		// Escape if all families are found
		if (indices.isComplete()) {
			break;
		}
	}

	return indices;
}