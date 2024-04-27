#include "BufferManager.hpp"
#include <stdexcept>
#include "QueueFamilies.hpp"
#include "CommandPool.hpp"

Device* BufferManager::device = nullptr;

void BufferManager::copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size) {
	if (!device) {
		throw("Error: device has not been initialized.");
	}

	VkCommandBuffer commandBuffer = device->commandPool->beginOneTimeCommand();

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;

	vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

	device->commandPool->endOneTimeCommand(commandBuffer);
}

void BufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, Buffer& buffer) {
	if (!device) {
		throw("Error: device has not been initialized.");
	}

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(*device, &bufferInfo, nullptr, &buffer.buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(*device, buffer.buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(*device, &allocInfo, nullptr, &buffer.memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(*device, buffer.buffer, buffer.memory, 0);
}

void BufferManager::destroyBuffer(Buffer& buffer) {
	if (!device) {
		throw("Error: device has not been initialized.");
	}

	vkDestroyBuffer(*device, buffer.buffer, nullptr);
	vkFreeMemory(*device, buffer.memory, nullptr);
}

uint32_t BufferManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	if (!device) {
		throw("Error: device has not been initialized.");
	}

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(device->physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}