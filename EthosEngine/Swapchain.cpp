#include "Swapchain.hpp"

#include "Engine.hpp"
#include "Instance.hpp"
#include "Surface.hpp"

#include "engineUtil.hpp"

#include <vector>
#include <algorithm>
#include "QueueFamilies.hpp"
#include "WindowWrapper.hpp"

#include "SyncManager.hpp"
#include "GraphicsPipeline.hpp"

std::vector<Swapchain::Frame> Swapchain::frames = {};
Swapchain::MetaFrameData Swapchain::metaFrameData;
VkExtent2D Swapchain::swapChainExtent = {};

Swapchain::SwapChainSupportDetails Swapchain::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
	SwapChainSupportDetails details;

	// Query Surface Capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	// Query number of supported formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	// Create a list of supported formats
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	// Query number of supported present modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	// Create a list of supported present modes
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	// Attempt to select SRGB format
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	// If unavailable, default to first format available
	return availableFormats[0];
}

#pragma warning(push)
#pragma warning(disable:26812)
VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	// Attempt to select mailbox present mode (Triple-buffer V-Sync)
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	// FIFO present mode is always available (V-Sync)
	return VK_PRESENT_MODE_FIFO_KHR;
}
#pragma warning(pop)

VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
	// If the surface only supports a matching resolution
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		// Retrieve window resoution
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		// Clamp height and width to within boundries of surface
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

#pragma warning(push)
#pragma warning(disable:26812)
void Swapchain::createImageViews() {
	// For each image in the swapchain
	for (size_t i = 0; i < frames.size(); i++) {
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = frames[i].image;

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &frames[i].imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}
#pragma warning(pop)

void Swapchain::init() {
	// Query swapchain support
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device.physicalDevice, *Engine::instance->surface);

	// Use helper functions to select optimal swapchain settings
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, *Engine::window);

	// Start at 1 more image than the minimum supported
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	// Create swapchain CreateInfo
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = *Engine::instance->surface;

	// Use pre-determined surface settings
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;

	// Start at 1 more image than the minimum supported
	createInfo.minImageCount = imageCount;
	// Clamp to max supported images; 0 meaning no limit
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	// If Queues for graphics and presentation match, use exclusive mode
	QueueFamilies::QueueFamilyIndices indices = QueueFamilies::findQueueFamilies(device.physicalDevice, *Engine::instance->surface);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	// Additional Swapchain Settings
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create the swapchain
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	// Query the number of images created
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);

	// Create a list of handles that reference the images created
	frames.resize(imageCount);

	std::vector<VkImage> tmp(imageCount);

	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, tmp.data());

	for (size_t i = 0; i < imageCount; ++i) {
		frames[i].image = tmp[i];
	}

	// Record swapchain format & extent
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	createImageViews();
}


void Swapchain::cleanup() {
	for (auto frame: frames) {
		vkDestroyImageView(device, frame.imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void Swapchain::recreate() {
	// Freeze while minimized
	int width = 0, height = 0;
	glfwGetFramebufferSize(*Engine::window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(*Engine::window, &width, &height);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(device);

	cleanup();
	init();

	DebugManager::message("Swapchain Recreated");

	graphicsPipeline->recreate();
}

void Swapchain::drawFrame() {
	graphicsPipeline->drawFrame();
}

Swapchain::Swapchain(Device& device) : device(device) {
	init();

	DebugManager::message("Swapchain Created");

	graphicsPipeline = new GraphicsPipeline(*this);
	syncManager = new SyncManager(*this);
}

Swapchain::~Swapchain() {
	delete syncManager;
	delete graphicsPipeline;
	cleanup();

	DebugManager::message("Swapchain Destroyed");
}