#pragma once
#include "pch.hpp"

class DescriptorManager {
public:
	static VkDescriptorSetLayout componentSpecificDescriptorSetLayout;
	static VkDescriptorSetLayout frameSpecificDescriptorSetLayout;
	//static VkDescriptorPool descriptorPool;

	static void destroyDescriptorPool(VkDescriptorPool& pool);

	static VkDescriptorPool createComponentSpecificDescriptorPool();
	static void createComponentSpecificDescriptorSetLayout();
	static VkDescriptorPool createFrameSpecificDescriptorPool();
	static void createFrameSpecificDescriptorSetLayout();
};