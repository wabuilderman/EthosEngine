#pragma once
#include "pch.hpp"

#include <vector>

#include "Surface.hpp"

class Swapchain;
class BufferManager;
class CommandPool;

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) ;
static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
static bool checkDeviceExtensionSupport(VkPhysicalDevice device);

class Device {
public:
	static VkQueue graphicsQueue;
	static VkQueue presentQueue;
	static VkDevice logicalDevice;
	static VkPhysicalDevice physicalDevice;

	static bool anistropyEnabled;

	// Child
	Swapchain* swapchain = nullptr;
	BufferManager* bufferManager = nullptr;
	CommandPool* commandPool = nullptr;

	operator const VkDevice&() const { return logicalDevice; }
	operator VkDevice&() { return logicalDevice; }

	void drawFrame();

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	Device();
	~Device();
};