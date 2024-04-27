#include "CommandPool.hpp"
#include "Surface.hpp"

VkCommandPool CommandPool::commandPool = VK_NULL_HANDLE;

CommandPool::CommandPool() {

	// Retrieve Queue Family Indices
	QueueFamilies::QueueFamilyIndices queueFamilyIndices = QueueFamilies::findQueueFamilies(Device::physicalDevice, Surface::surface);

	// Specify Command Pool Settings
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // Optional

	// Create Command Pool
	if (vkCreateCommandPool(Device::logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw "failed to create command pool!";
	}
}
CommandPool::~CommandPool() {
	vkDestroyCommandPool(Device::logicalDevice, commandPool, nullptr);
}

VkCommandBuffer CommandPool::beginOneTimeCommand() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(Device::logicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void CommandPool::endOneTimeCommand(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(Device::graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(Device::graphicsQueue);

	vkFreeCommandBuffers(Device::logicalDevice, commandPool, 1, &commandBuffer);
}