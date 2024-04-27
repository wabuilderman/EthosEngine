#pragma once
#include "pch.hpp"

#include "Device.hpp"
#include "Light.hpp"
#include <iostream>

class Buffer {
public:
	VkBuffer buffer;
	VkDeviceMemory memory;
};

class BufferManager {
public:
	// MUST be initialized before manager can be used
	static Device* device;

	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, Buffer& buffer);
	static void destroyBuffer(Buffer& buffer);
	static void copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size);
	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	static void writeMem(Buffer& buffer, void *dataSrc, VkDeviceSize size) {
		void* dataTmp;
		vkMapMemory(*device, buffer.memory, 0, size, 0, &dataTmp);
		memcpy(dataTmp, dataSrc, (size_t)size);
		vkUnmapMemory(*device, buffer.memory);
	}

	static void writeMemArray(Buffer& buffer, void *dataSrc, uint32_t count, VkDeviceSize size) {
		void* dataTmp;
		size_t totalSize = size * count + 4*sizeof(uint32_t);
		vkMapMemory(*device, buffer.memory, 0, (VkDeviceSize)totalSize, 0, &dataTmp);

		memcpy(dataTmp, &count, sizeof(uint32_t));
		memcpy(((char *)dataTmp) + 4*sizeof(uint32_t), dataSrc, (size_t)size * count);

		vkUnmapMemory(*device, buffer.memory);
	}
};