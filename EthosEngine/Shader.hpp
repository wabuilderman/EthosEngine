#pragma once
#include "pch.hpp"
#include <vector>
#include <fstream>
#include <string>
#include "DebugManager.hpp"

#pragma warning(push)
#pragma warning(disable: 26812)
class Shader {
	static void readFile(const std::string& filename, std::vector<char>& buffer) {
		DebugManager::message("Opening file '" + filename + "'");
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		buffer.resize(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
	}

	VkShaderModule& createShaderModule() {
		std::vector<char> code;
		readFile(sourcePath, code);

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(Device::logicalDevice, &createInfo, nullptr, &module) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return module;
	}

	void createStageCreateInfo(VkShaderStageFlagBits stageFlags) {
		stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageCreateInfo.stage = stageFlags;
		stageCreateInfo.module = module;
		stageCreateInfo.pName = "main";
	}

	VkPipelineShaderStageCreateInfo stageCreateInfo;
	VkShaderModule module;
	std::string sourcePath;

public:
	VkPipelineShaderStageCreateInfo& getStageCreateInfo() {
		return stageCreateInfo;
	}

	VkShaderModule& getModule() {
		return module;
	}



	Shader(const std::string& filename, const std::string& entrypoint, VkShaderStageFlagBits stageFlags) {
		sourcePath = filename;
		createShaderModule();
		createStageCreateInfo(stageFlags);
	}

	~Shader() {
		vkDestroyShaderModule(Device::logicalDevice, module, nullptr);
	}
};
#pragma warning(pop)