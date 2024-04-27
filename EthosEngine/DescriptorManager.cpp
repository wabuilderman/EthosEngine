#include "DescriptorManager.hpp"

#include <array>
#include "Swapchain.hpp"
#include "DebugManager.hpp"

VkDescriptorSetLayout DescriptorManager::componentSpecificDescriptorSetLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout DescriptorManager::frameSpecificDescriptorSetLayout = VK_NULL_HANDLE;

VkDescriptorPool DescriptorManager::createComponentSpecificDescriptorPool() {
	VkDescriptorPool pool;

	VkDescriptorPoolSize poolSizes[] = {{
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = static_cast<uint32_t>(Swapchain::frames.size()) * 2
	},{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = static_cast<uint32_t>(Swapchain::frames.size()) * 1
	},{
		.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = static_cast<uint32_t>(Swapchain::frames.size()) * 5
	}};

	VkDescriptorPoolCreateInfo poolInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = static_cast<uint32_t>(Swapchain::frames.size()),
		.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes)),
		.pPoolSizes = poolSizes
	};

	if (vkCreateDescriptorPool(Device::logicalDevice, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	return pool;
}

VkDescriptorPool DescriptorManager::createFrameSpecificDescriptorPool() {
	VkDescriptorPool pool;

	VkDescriptorPoolSize poolSizes[] = { {
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = static_cast<uint32_t>(Swapchain::frames.size())
	}};

	VkDescriptorPoolCreateInfo poolInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = static_cast<uint32_t>(Swapchain::frames.size()),
		.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes)),
		.pPoolSizes = poolSizes
	};
	
	if (vkCreateDescriptorPool(Device::logicalDevice, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
	return pool;
}

void DescriptorManager::destroyDescriptorPool(VkDescriptorPool& pool) {
	vkDestroyDescriptorPool(Device::logicalDevice, pool, nullptr);
}

void DescriptorManager::createFrameSpecificDescriptorSetLayout() {

	VkDescriptorSetLayoutBinding bindings[] = {{
		.binding = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr
	}};

	VkDescriptorSetLayoutCreateInfo layoutInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(std::size(bindings)),
		.pBindings = bindings
	};

	if (vkCreateDescriptorSetLayout(Device::logicalDevice, &layoutInfo, nullptr, &frameSpecificDescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void DescriptorManager::createComponentSpecificDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding bindings[] = {{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = nullptr
	},{
		.binding = 2,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr
	},{
		.binding = 3,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr
	},{
		.binding = 5,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr
	},{
		.binding = 6,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr
	},{
		.binding = 7,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr
	},{
		.binding = 4,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr
	},{
		.binding = 8,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = nullptr
	}
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(std::size(bindings)),
		.pBindings = bindings
	};

	if (vkCreateDescriptorSetLayout(Device::logicalDevice, &layoutInfo, nullptr, &componentSpecificDescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}