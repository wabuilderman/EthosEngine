#pragma once
#include "pch.hpp"

#include <vector>
#include "QueueFamilies.hpp"
#include "Device.hpp"
#include "BufferManager.hpp"

class SyncManager;
class GraphicsPipeline;

class Swapchain {
public:
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;

	struct Frame {
		VkImage image;
		VkImageView imageView;
		VkFramebuffer framebuffer;

		struct FrameData {
			std::vector<Buffer> MatBuffers;
			std::vector<Buffer> BoneBuffers;
			std::vector<Buffer> UBOs;
			std::vector<Buffer> UI_UBOs;
			std::vector<Buffer> DebugVisualizer_UBOs;

			Buffer lightsBuffer = Buffer();
			unsigned lightCount = 0;

			std::vector<VkDescriptorSet> componentSpecificDescriptorSets;
			std::vector<VkDescriptorSet> UI_componentSpecificDescriptorSets;
			std::vector<VkDescriptorSet> DebugVisualizer_componentSpecificDescriptorSets;
		} data;
	};

	struct MetaFrameData {
		std::vector<VkDescriptorPool> componentSpecificDescriptorPool;
		std::vector<VkDescriptorPool> UI_componentSpecificDescriptorPool;
		std::vector<VkDescriptorPool> DebugVisualizer_componentSpecificDescriptorPool;

		VkDescriptorPool frameSpecificDescriptorPool = nullptr;
		std::vector<VkDescriptorSet> frameSpecificDescriptorSets;
	};

	static MetaFrameData metaFrameData;
	static std::vector<Frame> frames;

	//std::vector<VkImage> swapChainImages;
	//std::vector<VkImageView> swapChainImageViews;

	//static size_t imageViewCount;

	VkFormat swapChainImageFormat;
	static VkExtent2D swapChainExtent;

	Device& device;

	SyncManager* syncManager = nullptr;
	GraphicsPipeline* graphicsPipeline = nullptr;

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities = {};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

private:
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

	void createImageViews();

	
	

public:
	operator const VkSwapchainKHR&() const { return swapchain; }
	operator VkSwapchainKHR&() { return swapchain; }

	Swapchain(Device& device);
	void cleanup();
	void init();
	void recreate();
	void drawFrame();
	~Swapchain();
};