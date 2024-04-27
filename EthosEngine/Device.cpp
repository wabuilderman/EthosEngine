#include "Device.hpp"

#include "Engine.hpp"
#include "engineUtil.hpp"

#include "Instance.hpp"
#include "Surface.hpp"

#include <vector>
#include <set>

#include "Instance.hpp"
#include "QueueFamilies.hpp"
#include "Swapchain.hpp"
#include "SyncManager.hpp"
#include "BufferManager.hpp"
#include "CommandPool.hpp"

#include "BufferManager.hpp"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkQueue Device::graphicsQueue = VK_NULL_HANDLE;
VkQueue Device::presentQueue = VK_NULL_HANDLE;
VkDevice Device::logicalDevice = VK_NULL_HANDLE;
VkPhysicalDevice Device::physicalDevice = VK_NULL_HANDLE;
bool Device::anistropyEnabled = true;

void Device::drawFrame() {
	swapchain->drawFrame();
}

static VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	// Query device count
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// If no devices are found
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	// Create a list of devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// Check each device for suitablity
	for (const auto& device : devices) {
		if (isDeviceSuitable(device, surface)) {
			physicalDevice = device;
			break;
		}
	}

	// If no suitable devices are found
	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	return physicalDevice;
}

static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
	// Check Queue Family support
	QueueFamilies::QueueFamilyIndices indices = QueueFamilies::findQueueFamilies(device, surface);

	// Check Device Extension support
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	// Check for suitable swapchain support
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		Swapchain::SwapChainSupportDetails swapChainSupport = Swapchain::querySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

static bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

#pragma warning(push)
#pragma warning(disable: 26812)
VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}
#pragma warning(pop)

Device::Device() {
	physicalDevice = pickPhysicalDevice(*Engine::instance, *Engine::instance->surface);

	// Get the indices of queue families which support desired features
	QueueFamilies::QueueFamilyIndices indices = QueueFamilies::findQueueFamilies(physicalDevice, *Engine::instance->surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	// Fill CreateInfo structs for each queue
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

	// Specify Device Features
	VkPhysicalDeviceFeatures deviceFeatures = {};

	if (supportedFeatures.samplerAnisotropy) {
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		anistropyEnabled = true;
	}

	// Define Logical Device CreateInfo
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledLayerCount = 0;

	// Enable extensions
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// Legacy Device-Specific Validation 
	if(enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	// Create Logical Device
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	// Get a reference to the graphics queue that has been implicitly created
	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
	// Get a reference to the present queue that has been implicitly created
	vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);

	DebugManager::message("Device Created");
	commandPool = new CommandPool();
	BufferManager::device = this;

	swapchain = new Swapchain(*this);
}


Device::~Device() {
	delete swapchain;
	//delete bufferManager;
	delete commandPool;
	vkDestroyDevice(logicalDevice, nullptr);

	DebugManager::message("Device Destroyed");
}