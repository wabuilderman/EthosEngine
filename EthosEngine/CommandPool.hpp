#pragma once
#include "pch.hpp"

#include "Device.hpp"
#include "QueueFamilies.hpp"

class CommandPool {
public:
	static VkCommandPool commandPool;

	operator const VkCommandPool&() const { return commandPool; }
	operator VkCommandPool&() { return commandPool; }

	CommandPool();
	~CommandPool();

	static VkCommandBuffer beginOneTimeCommand();

	static void endOneTimeCommand(VkCommandBuffer commandBuffer);
};
