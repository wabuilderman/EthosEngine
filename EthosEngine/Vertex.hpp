#pragma once
#include "pch.hpp"

#include <array>

struct Vertex {
	glm::vec3 pos = { 0, 0, 0 };
	glm::vec3 normal = { 0, 1, 0 };
	glm::vec3 tangent = { 0, 0, 1 };
	glm::vec4 color = { 0, 0, 0, 1 };
	glm::vec2 texCoord = { 0, 0 };

	glm::vec4 bone_indices = { -1, -1, -1, -1 };
	//glm::vec4 bone_weights = { 0, 0, 0, 0 };

	static constexpr unsigned max_influences = 4;

	static VkVertexInputBindingDescription& getBindingDescription() {
		static VkVertexInputBindingDescription bindingDescription = {
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};

		return bindingDescription;
	}

#pragma warning(push)
#pragma warning(disable:26812)
	static std::array<VkVertexInputAttributeDescription, 6>& getAttributeDescriptions() {
		static std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions = {
			VkVertexInputAttributeDescription({
				.location = 0,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, pos)
			}),
			VkVertexInputAttributeDescription({
				.location = 1,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, normal)
			}),
			VkVertexInputAttributeDescription({
				.location = 2,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, tangent)
			}),
			VkVertexInputAttributeDescription({
				.location = 3,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32A32_SFLOAT,
				.offset = offsetof(Vertex, color)
			}),
			VkVertexInputAttributeDescription({
				.location = 4,
				.binding = 0,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = offsetof(Vertex, texCoord)
			}),VkVertexInputAttributeDescription({
				.location = 5,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32A32_SFLOAT,
				.offset = offsetof(Vertex, bone_indices)
			})///,VkVertexInputAttributeDescription({
				//.location = 6,
				//.binding = 0,
				//.format = VK_FORMAT_R32G32B32A32_SFLOAT,
				//.offset = offsetof(Vertex, bone_weights)
			//})
		};
		return attributeDescriptions;
	}
#pragma warning(pop)
};