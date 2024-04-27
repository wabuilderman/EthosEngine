#pragma once
#include "pch.hpp"

class ImageResource {
public:
	VkImage image;
	VkDeviceMemory memory;
	VkImageView imageView;

	ImageResource(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectMask);
	~ImageResource();

	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

	void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height);

	static bool hasStencilComponent(VkFormat format);
};