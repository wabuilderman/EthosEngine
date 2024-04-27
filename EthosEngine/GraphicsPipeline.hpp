#pragma once
#include "pch.hpp"

#include <vector>
#include <fstream>

#include "Swapchain.hpp"
#include "Vertex.hpp"
#include "BufferManager.hpp"
#include "ImageResource.hpp"

class RenderComponentManager;

class GraphicsPipeline {
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;

	VkCommandPool commandPool;

	std::vector<VkCommandBuffer> commandBuffers;

	//std::vector<VkFramebuffer> swapChainFramebuffers;
	ImageResource* depthImage;

	void init();
	void cleanup();
	
	void createCommandPool();

	void createRenderPass();
	void createCommandBuffers();

	void recordCommandBuffer(size_t index);
	void createCommandBuffer(size_t index);
	void createFramebuffers();
	
	void createGraphicsPipeline();
	void createGraphicsPipelineLayout();

public:
	Swapchain& swapchain;
	std::array<VkPipeline, 3> graphicsPipelines;

	void rerecordCommandBuffers();
	void rerecordCommandBuffer(size_t index);

	void createDepthResources();
	VkFormat findDepthFormat();

	void recreate();
	void drawFrame();

	GraphicsPipeline(Swapchain& swapchain);
	~GraphicsPipeline();
};