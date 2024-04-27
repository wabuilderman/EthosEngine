#include "RenderComponentManager.hpp"
#include "Swapchain.hpp"
#include "DescriptorManager.hpp"
#include "DebugManager.hpp"
#include "Model.hpp"
#include "Actor.hpp"

#include "CameraManager.hpp"

#include "Light.hpp"
#include <chrono>

#include "UpdateEvent.hpp"
#include "Message.hpp"

#include "SyncManager.hpp"

#include "Armature.hpp"

#include <unordered_map>
#include <algorithm>

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) glm::mat4 norm;
	alignas(16) glm::vec4 color;
	alignas(16) glm::vec3 camPos;
	alignas(16) glm::vec2 uvScale;
};

struct MaterialBuffer {
	alignas(16) glm::vec3 baseColor = { 1, 1, 1 };
	alignas(16) glm::vec4 emission = { 0, 0, 0, 0 };
	alignas(4) float metallic = 0.0f;
	alignas(4) float specular = 0.5f;
	alignas(4) float roughness = 0.4f;
	alignas(4) float sheen = 0.0f;
	alignas(4) float IOR = 1.45f;
	alignas(8) glm::vec2 diffuseUVScale = { 0, 0 };
	alignas(8) glm::vec2 normalUVScale = { 0, 0 };
	alignas(8) glm::vec2 specularUVScale = { 0, 0 };
	alignas(8) glm::vec2 ambientUVScale = { 0, 0 };
	alignas(8) glm::vec2 displaceUVScale = { 0, 0 };
};

struct LightStorageBuffer {
	unsigned int count;
	Light* lights;
};

struct BoneBuffer {
	unsigned int count;
	glm::mat4* bones;
};

enum class ComponentIndexType {
	standard,
	ui,
	debugVisual
};



std::vector<RenderComponentManager::LightID> lightIDs;
std::vector<Light> lights;

size_t lightBufferSize() {
	return sizeof(Light) * lights.size() + 4*sizeof(uint32_t);
}

GraphicsPipeline* RenderComponentManager::graphicsPipeline = nullptr;

std::vector<RenderComponent*> RenderComponentManager::components;
std::vector<UIComponent*> RenderComponentManager::uiComponents;
std::vector<DebugVisualizerComponent*> RenderComponentManager::debugVisualizerComponents;

std::vector<Buffer> RenderComponentManager::VBOs;
std::vector<Buffer> RenderComponentManager::IBOs;

#pragma warning(push)
#pragma warning(disable: 26812)
constexpr VkWriteDescriptorSet writeDescriptorSetBuffer(
	VkDescriptorSet descriptorSet, uint32_t binding,
	uint32_t arrayElement, uint32_t count, VkDescriptorType type,
	const VkDescriptorBufferInfo& bufferInfo
) {
	return VkWriteDescriptorSet({
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = descriptorSet,
		.dstBinding = binding,
		.dstArrayElement = arrayElement,
		.descriptorCount = count,
		.descriptorType = type,
		.pBufferInfo = &bufferInfo
		});
};
constexpr VkWriteDescriptorSet writeDescriptorSetImage(
	VkDescriptorSet descriptorSet, uint32_t binding,
	uint32_t arrayElement, uint32_t count, VkDescriptorType type,
	const VkDescriptorImageInfo& imageInfo
) {
	return VkWriteDescriptorSet({
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = descriptorSet,
		.dstBinding = binding,
		.dstArrayElement = arrayElement,
		.descriptorCount = count,
		.descriptorType = type,
		.pImageInfo = &imageInfo
		});
};
#pragma warning(pop)

void allocFrameSpecificDescriptors(VkDescriptorPool& pool, std::vector<VkDescriptorSet>& sets, VkDescriptorSetLayout layout) {
	pool = DescriptorManager::createFrameSpecificDescriptorPool();

	std::vector<VkDescriptorSetLayout> layouts(Swapchain::frames.size(), layout);

	VkDescriptorSetAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = pool,
		.descriptorSetCount = (uint32_t)Swapchain::frames.size(),
		.pSetLayouts = layouts.data()
	};

	sets.resize(Swapchain::frames.size());

	if (vkAllocateDescriptorSets(Device::logicalDevice, &allocInfo, sets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate frame-specific descriptor sets!");
	}
}

void writeFrameSpecificDescriptors(size_t index) {
	VkDescriptorBufferInfo storageBufferInfo = {
			.buffer = Swapchain::frames[index].data.lightsBuffer.buffer,
			.offset = 0,
			.range = (uint32_t)(lightBufferSize())
	};

	VkWriteDescriptorSet descriptorWrites[] = {
		writeDescriptorSetBuffer(Swapchain::metaFrameData.frameSpecificDescriptorSets[index], 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, storageBufferInfo),
	};

	vkUpdateDescriptorSets(
		Device::logicalDevice,
		static_cast<uint32_t>(std::size(descriptorWrites)),
		descriptorWrites, 0, nullptr
	);
}

void writeFrameSpecificDescriptors() {
	for (size_t i = 0; i < Swapchain::frames.size(); ++i) {
		writeFrameSpecificDescriptors(i);
	}
}

void allocComponentSpecificDescriptorSets(size_t count, VkDescriptorPool pool, VkDescriptorSet* sets, VkDescriptorSetLayout layout) {
	std::vector<VkDescriptorSetLayout> layouts(count, layout);
	VkDescriptorSetAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = pool,
		.descriptorSetCount = static_cast<uint32_t>(count),
		.pSetLayouts = layouts.data()
	};

	if (vkAllocateDescriptorSets(Device::logicalDevice, &allocInfo, sets) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate component-specific descriptor sets!");
	}
}

RenderComponentManager::LightID RenderComponentManager::addLight(const Light& light) {
	static LightID id = 0;
	id++;

	lights.push_back(light);
	lightIDs.push_back(id);

	/*
	for (size_t i = 0; i < Swapchain::frames.size(); ++i) {
		destroyLightBuffer(Swapchain::frames[i].data.lightsBuffer);
		createLightBuffer(Swapchain::frames[i].data.lightsBuffer);
	}
	writeFrameSpecificDescriptors();
	*/

	return id;
}

Light* RenderComponentManager::getLight(LightID light) {
	for (size_t i = 0; i < lightIDs.size(); ++i) {
		if (lightIDs[i] == light) {
			return &lights[i];
		}
	}
	return nullptr;
}

void RenderComponentManager::removeLight(LightID light) {
	for (size_t i = 0; i < lightIDs.size(); ++i) {
		if (lightIDs[i] == light) {
			lights.erase(lights.begin() + i);
			lightIDs.erase(lightIDs.begin() + i);
			break;
		}
	}
}

void RenderComponentManager::init(GraphicsPipeline* gp) {
	graphicsPipeline = gp;
	Texture::init();
	
	for (Swapchain::Frame& frame : Swapchain::frames) {
		createLightBuffer(frame.data.lightsBuffer);
		updateLightBuffer(frame.data.lightsBuffer);
	}

	allocFrameSpecificDescriptors(
		Swapchain::metaFrameData.frameSpecificDescriptorPool,
		Swapchain::metaFrameData.frameSpecificDescriptorSets,
		DescriptorManager::frameSpecificDescriptorSetLayout
	);

	writeFrameSpecificDescriptors();

	Light mySun = {
		.pos = { 0, 1, 0 },
		.color = {1.0f, 0.9f, 0.9f, 0.7f},
		.dir = {0, 1, -1},
		.type = Light::Type::Sun
	};

	addLight(mySun);
}

void RenderComponentManager::cleanup() {

	for (size_t i = 0; i < VBOs.size(); ++i) {
		destroyVertexBuffer(VBOs[i]);
	}
	for (size_t i = 0; i < IBOs.size(); ++i) {
		destroyIndexBuffer(IBOs[i]);
	}

	for (Swapchain::Frame frame : Swapchain::frames) {

		destroyLightBuffer(frame.data.lightsBuffer);

		for (Buffer& buffer : frame.data.UBOs) {
			destroyUniformBuffer(buffer);
		}

		for (Buffer& buffer : frame.data.BoneBuffers) {
			destroyBoneBuffer(buffer);
		}

		for (Buffer& buffer : frame.data.UI_UBOs) {
			destroyUniformBuffer(buffer);
		}
	}

	for (VkDescriptorPool pool : Swapchain::metaFrameData.UI_componentSpecificDescriptorPool) {
		DescriptorManager::destroyDescriptorPool(pool);
	}
	for (VkDescriptorPool pool : Swapchain::metaFrameData.componentSpecificDescriptorPool) {
		DescriptorManager::destroyDescriptorPool(pool);
	}

	DescriptorManager::destroyDescriptorPool(Swapchain::metaFrameData.frameSpecificDescriptorPool);

	delete Texture::defaultTexture;
}

std::vector<std::pair<Mesh*, size_t>> meshIndices;

void RenderComponentManager::addMesh(Mesh* mesh) {
	// first, find if this mesh already exists, then, if it it does, increment the number of uses and return the index.
	for (size_t i = 0; i < meshIndices.size(); ++i) {
		if (meshIndices[i].first == mesh) {
			meshIndices[i].second++;
			return;
		}
	}

	// Otherwise, add the mesh to the list with a use of 1.
	meshIndices.push_back(std::pair<Mesh*, size_t>(mesh, 1));

	// Create VBO
	VBOs.push_back(Buffer());
	createVertexBuffer(VBOs.back(), *mesh);

	// Create IBO
	IBOs.push_back(Buffer());
	createIndexBuffer(IBOs.back(), *mesh);
}

void RenderComponentManager::updateMesh(Mesh* mesh) {
	for (size_t i = 0; i < meshIndices.size(); ++i) {
		if (meshIndices[i].first == mesh) {
			updateVertexBuffer(VBOs[i], *mesh);
		}
	}
}

void RenderComponentManager::removeMesh(Mesh* mesh) {
	for (size_t i = 0; i < meshIndices.size(); ++i) {
		if (meshIndices[i].first == mesh) {
			// decrement the number of uses, then, if there are no more uses, remove the mesh data.
			if (--meshIndices[i].second == 0) {
				destroyVertexBuffer(VBOs[i]);
				VBOs.erase(VBOs.begin() + i);

				destroyIndexBuffer(IBOs[i]);
				IBOs.erase(IBOs.begin() + i);

				meshIndices.erase(meshIndices.begin() + i);
			}
			return;
		}
	}
}

size_t RenderComponentManager::getMeshIndex(Mesh* mesh) {
	for (size_t i = 0; i < meshIndices.size(); ++i) {
		if (meshIndices[i].first == mesh) {
			return i;
		}
	}
	throw std::runtime_error("Mesh does not exist");
}

void RenderComponentManager::addUIComponent(UIComponent* component) {
	// Check for identical component
	for (size_t i = 0; i < uiComponents.size(); ++i) {
		if (uiComponents[i] == component) {
			throw("Component Already Exists!");
		}
	}

	addMesh(component->model->mesh);

	// Create UBOs
	uiComponents.push_back(component);
	Swapchain::metaFrameData.UI_componentSpecificDescriptorPool.push_back(DescriptorManager::createComponentSpecificDescriptorPool());

	for (size_t i = 0; i < Swapchain::frames.size(); ++i) {
		Swapchain::frames[i].data.UI_UBOs.push_back(Buffer());
		createUniformBuffer(Swapchain::frames[i].data.UI_UBOs.back());

		Swapchain::frames[i].data.UI_componentSpecificDescriptorSets.push_back(VkDescriptorSet());
		allocComponentSpecificDescriptorSets(
			1,
			Swapchain::metaFrameData.UI_componentSpecificDescriptorPool.back(),
			&Swapchain::frames[i].data.UI_componentSpecificDescriptorSets.back(), 
			DescriptorManager::componentSpecificDescriptorSetLayout
		);

		createUIDescriptorSets(Swapchain::frames[i], uiComponents.size() - 1);

		vkDeviceWaitIdle(Device::logicalDevice);
		graphicsPipeline->rerecordCommandBuffer(i);
	}
}

void RenderComponentManager::updateUIComponentVBO(UIComponent* component) {
	size_t componentIndex = uiComponents.size();
	for (size_t i = 0; i < uiComponents.size(); ++i) {
		if (uiComponents[i] == component) {
			componentIndex = i;
			break;
		}
	}

	if (componentIndex == uiComponents.size()) {
		throw "Error: UI Component does not exist";
	}

	updateMesh(component->model->mesh);

	for (size_t j = 0; j < Swapchain::frames.size(); ++j) {
		updateUniformBuffer(*component, Swapchain::frames[j].data.UI_UBOs[componentIndex]);
		vkDeviceWaitIdle(Device::logicalDevice);
		graphicsPipeline->rerecordCommandBuffer(j);
	}
}

void RenderComponentManager::removeUIComponent(UIComponent* component) {
	size_t indexWithinComponentList = 0;
	bool found = false;
	for (size_t i = 0; i < uiComponents.size(); ++i) {
		if (uiComponents[i] == component) {
			found = true;
			indexWithinComponentList = i;
		}
	}
	if (!found) {
		throw("Component Never Existed!");
	}

	removeMesh(component->model->mesh);

	for (Swapchain::Frame& frame : Swapchain::frames) {
		destroyUniformBuffer(frame.data.UI_UBOs[indexWithinComponentList]);
		frame.data.UI_UBOs.erase(frame.data.UI_UBOs.begin() + indexWithinComponentList);
		frame.data.UI_componentSpecificDescriptorSets
			.erase(frame.data.UI_componentSpecificDescriptorSets.begin() + indexWithinComponentList);
	}

	DescriptorManager::destroyDescriptorPool(Swapchain::metaFrameData.UI_componentSpecificDescriptorPool[indexWithinComponentList]);
	Swapchain::metaFrameData.UI_componentSpecificDescriptorPool.erase(Swapchain::metaFrameData.UI_componentSpecificDescriptorPool.begin() + indexWithinComponentList);

	uiComponents.erase(uiComponents.begin() + indexWithinComponentList);

	for (int j = 0; j < Swapchain::frames.size(); ++j) {
		vkDeviceWaitIdle(Device::logicalDevice);
		graphicsPipeline->rerecordCommandBuffer(j);
	}
}

void RenderComponentManager::addDebugVisualizerComponent(DebugVisualizerComponent* component) {
	// Check for identical component
	for (size_t i = 0; i < debugVisualizerComponents.size(); ++i) {
		if (debugVisualizerComponents[i] == component) {
			throw("Component Already Exists!");
		}
	}

	// Check for shared mesh
	size_t meshIndex = debugVisualizerComponents.size();
	for (size_t i = 0; i < debugVisualizerComponents.size(); ++i) {
		if (debugVisualizerComponents[i]->model->mesh == component->model->mesh) {
			meshIndex = i;
			break;
		}
	}

	addMesh(component->model->mesh);

	// Create UBOs
	debugVisualizerComponents.push_back(component);
	Swapchain::metaFrameData.DebugVisualizer_componentSpecificDescriptorPool.push_back(DescriptorManager::createComponentSpecificDescriptorPool());

	for (size_t i = 0; i < Swapchain::frames.size(); ++i) {
		Swapchain::frames[i].data.DebugVisualizer_UBOs.push_back(Buffer());
		createUniformBuffer(Swapchain::frames[i].data.DebugVisualizer_UBOs.back());

		Swapchain::frames[i].data.DebugVisualizer_componentSpecificDescriptorSets.push_back(VkDescriptorSet());
		allocComponentSpecificDescriptorSets(
			1,
			Swapchain::metaFrameData.DebugVisualizer_componentSpecificDescriptorPool.back(),
			&Swapchain::frames[i].data.DebugVisualizer_componentSpecificDescriptorSets.back(),
			DescriptorManager::componentSpecificDescriptorSetLayout
		);

		createUIDescriptorSets(Swapchain::frames[i], debugVisualizerComponents.size() - 1);

		vkDeviceWaitIdle(Device::logicalDevice);
		graphicsPipeline->rerecordCommandBuffer(i);
	}
}

void RenderComponentManager::updateDebugVisualizerComponentVBO(DebugVisualizerComponent* component) {
	size_t componentIndex = debugVisualizerComponents.size();
	for (size_t i = 0; i < debugVisualizerComponents.size(); ++i) {
		if (debugVisualizerComponents[i] == component) {
			componentIndex = i;
			break;
		}
	}

	if (componentIndex == debugVisualizerComponents.size()) {
		throw "Error: UI Component does not exist";
	}

	updateMesh(component->model->mesh);

	for (size_t j = 0; j < Swapchain::frames.size(); ++j) {
		updateUniformBuffer(*component, Swapchain::frames[j].data.DebugVisualizer_UBOs[componentIndex]);
		vkDeviceWaitIdle(Device::logicalDevice);
		graphicsPipeline->rerecordCommandBuffer(j);
	}
}

void RenderComponentManager::removeDebugVisualizerComponent(DebugVisualizerComponent* component) {
	for (size_t i = 0; i < debugVisualizerComponents.size(); ++i) {
		if (debugVisualizerComponents[i] == component) {

			removeMesh(component->model->mesh);

			for (size_t j = 0; j < Swapchain::frames.size(); ++j) {
				destroyUniformBuffer(Swapchain::frames[j].data.DebugVisualizer_UBOs[i]);
				Swapchain::frames[j].data.DebugVisualizer_UBOs.erase(Swapchain::frames[j].data.DebugVisualizer_UBOs.begin() + i);
				Swapchain::frames[j].data.DebugVisualizer_componentSpecificDescriptorSets
					.erase(Swapchain::frames[j].data.DebugVisualizer_componentSpecificDescriptorSets.begin() + i);
			}

			DescriptorManager::destroyDescriptorPool(Swapchain::metaFrameData.DebugVisualizer_componentSpecificDescriptorPool[i]);
			Swapchain::metaFrameData.DebugVisualizer_componentSpecificDescriptorPool.erase(Swapchain::metaFrameData.DebugVisualizer_componentSpecificDescriptorPool.begin() + i);

			debugVisualizerComponents.erase(debugVisualizerComponents.begin() + i);
			return;
		}
	}
	throw("Component Never Existed!");
}

void RenderComponentManager::zSortComponents(uint32_t currentImage) {
	size_t numTransparent = 0;

	for (size_t i = 0; i < components.size() - numTransparent; ++i) {
		if (!components[i]->model->material->isOpaque) {
			// Z-sort this component!
			numTransparent++;

			i--;
		}
	}
}

void RenderComponentManager::addRenderComponent(RenderComponent* component) {
	// TODO: Consider another structure besides vector
	for (size_t i = 0; i < components.size(); ++i) {
		if (components[i] == component) {
			throw("Component Already Exists!");
		}
	}
	components.push_back(component);

	addMesh(component->model->mesh);

	// Create UBOs
	Swapchain::metaFrameData.componentSpecificDescriptorPool.push_back(DescriptorManager::createComponentSpecificDescriptorPool());
	for (size_t i = 0; i < Swapchain::frames.size(); ++i) {
		Swapchain::frames[i].data.UBOs.push_back(Buffer());
		Swapchain::frames[i].data.MatBuffers.push_back(Buffer());
		Swapchain::frames[i].data.BoneBuffers.push_back(Buffer());

		createUniformBuffer(Swapchain::frames[i].data.UBOs.back());
		createMaterialBuffer(Swapchain::frames[i].data.MatBuffers.back());
		createBoneBuffer(Swapchain::frames[i].data.BoneBuffers.back(), components.back()->model->armature->getBoneMatrices().size());

		Swapchain::frames[i].data.componentSpecificDescriptorSets.push_back(VkDescriptorSet());
		allocComponentSpecificDescriptorSets(
			1,
			Swapchain::metaFrameData.componentSpecificDescriptorPool.back(),
			&Swapchain::frames[i].data.componentSpecificDescriptorSets.back(),
			DescriptorManager::componentSpecificDescriptorSetLayout
		);

		createDescriptorSets(Swapchain::frames[i], components.size() - 1);

		vkDeviceWaitIdle(Device::logicalDevice);
		graphicsPipeline->rerecordCommandBuffer(i);
	}
}

void RenderComponentManager::removeRenderComponent(RenderComponent* component) {
	for (size_t i = 0; i < components.size(); ++i) {
		if (components[i] == component) {
			removeMesh(component->model->mesh); // marks one less active use of the mesh

			for (int j = 0; j < Swapchain::frames.size(); ++j) {
				// Destroy Vulkan resources associated with the component
				destroyUniformBuffer(Swapchain::frames[j].data.UBOs[i]);
				destroyMaterialBuffer(Swapchain::frames[j].data.MatBuffers[i]);
				destroyBoneBuffer(Swapchain::frames[j].data.BoneBuffers[i]);

				// Erase the corresponding elements
				Swapchain::frames[j].data.UBOs.erase(Swapchain::frames[j].data.UBOs.begin() + i);
				Swapchain::frames[j].data.MatBuffers.erase(Swapchain::frames[j].data.MatBuffers.begin() + i);
				Swapchain::frames[j].data.BoneBuffers.erase(Swapchain::frames[j].data.BoneBuffers.begin() + i);

				// Free descriptor set
				vkFreeDescriptorSets(
					Device::logicalDevice,
					Swapchain::metaFrameData.componentSpecificDescriptorPool[i],
					1,
					&Swapchain::frames[j].data.componentSpecificDescriptorSets[i]
				);

				// Erase descriptor set
				Swapchain::frames[j].data.componentSpecificDescriptorSets.erase(
					Swapchain::frames[j].data.componentSpecificDescriptorSets.begin() + i
				);
			}

			DescriptorManager::destroyDescriptorPool(
				Swapchain::metaFrameData.componentSpecificDescriptorPool[i]
			);

			Swapchain::metaFrameData.componentSpecificDescriptorPool.erase(
				Swapchain::metaFrameData.componentSpecificDescriptorPool.begin() + i
			);

			components.erase(components.begin() + i);

			for (int j = 0; j < Swapchain::frames.size(); ++j) {
				vkDeviceWaitIdle(Device::logicalDevice);
				graphicsPipeline->rerecordCommandBuffer(j);
			}

			return;
		}
	}
	throw(std::runtime_error("Component Never Existed!"));
}

void RenderComponentManager::updateRenderComponentVisibility(RenderComponent* component, bool visible) {
	component->isVisible = visible;
	graphicsPipeline->rerecordCommandBuffers();
}

void RenderComponentManager::recreate() {
	// Destroy Frame Data
	for (auto& frame : Swapchain::frames) {
		destroyLightBuffer(frame.data.lightsBuffer);

		for (size_t i = 0; i < components.size(); ++i) {
			destroyUniformBuffer(frame.data.UBOs[i]);
			destroyMaterialBuffer(frame.data.MatBuffers[i]);
		}

		for (size_t i = 0; i < uiComponents.size(); ++i) {
			destroyUniformBuffer(frame.data.UI_UBOs[i]);
		}
	}

	// Create Frame Data
	for (auto& frame : Swapchain::frames) {

		createLightBuffer(frame.data.lightsBuffer);;

		for (size_t i = 0; i < components.size(); ++i) {
			createMaterialBuffer(frame.data.MatBuffers[i]);
			createUniformBuffer(frame.data.UBOs[i]);

			frame.data.componentSpecificDescriptorSets.resize(components.size());
			allocComponentSpecificDescriptorSets(
				components.size(),
				Swapchain::metaFrameData.componentSpecificDescriptorPool[i],
				frame.data.componentSpecificDescriptorSets.data(),
				DescriptorManager::componentSpecificDescriptorSetLayout
			);

			createDescriptorSets(frame, i);
		}

		for (size_t i = 0; i < uiComponents.size(); ++i) {
			createUniformBuffer(frame.data.UI_UBOs[i]);

			frame.data.UI_componentSpecificDescriptorSets.resize(uiComponents.size());
			allocComponentSpecificDescriptorSets(
				uiComponents.size(),
				Swapchain::metaFrameData.UI_componentSpecificDescriptorPool[i],
				frame.data.UI_componentSpecificDescriptorSets.data(),
				DescriptorManager::componentSpecificDescriptorSetLayout
			);

			createUIDescriptorSets(frame, i);
		}
	}

	allocFrameSpecificDescriptors(
		Swapchain::metaFrameData.frameSpecificDescriptorPool,
		Swapchain::metaFrameData.frameSpecificDescriptorSets,
		DescriptorManager::frameSpecificDescriptorSetLayout
	);

	writeFrameSpecificDescriptors();
}

void RenderComponentManager::updateAllUniformBuffers(uint32_t currentImage) {
	auto& curFrameData = Swapchain::frames[currentImage].data;

	if (curFrameData.lightCount != lights.size()) {
		destroyLightBuffer(curFrameData.lightsBuffer);
		createLightBuffer(curFrameData.lightsBuffer);
		curFrameData.lightCount = static_cast<unsigned>(lights.size());

		writeFrameSpecificDescriptors(currentImage);

		vkDeviceWaitIdle(Device::logicalDevice);
		graphicsPipeline->rerecordCommandBuffer(currentImage);
	}

	updateLightBuffer(curFrameData.lightsBuffer);

	for (size_t i = 0; i < components.size(); ++i) {
		updateUniformBuffer(*(components[i]), curFrameData.UBOs[i]);
		updateMaterialBuffer(*(components[i]), curFrameData.MatBuffers[i]);
		updateBoneBuffer(*(components[i]), curFrameData.BoneBuffers[i]);
	}

	for (size_t i = 0; i < uiComponents.size(); ++i) {
		updateUniformBuffer(*(uiComponents[i]), curFrameData.UI_UBOs[i]);
	}
}

void RenderComponentManager::updateVertexBuffer(Buffer& VBO, Mesh& mesh) {
	size_t bufferSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();

	Buffer stagingBuffer;
	BufferManager::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);
	BufferManager::writeMem(stagingBuffer, mesh.vertices.data(), bufferSize);
	BufferManager::copyBuffer(stagingBuffer, VBO, bufferSize);
	BufferManager::destroyBuffer(stagingBuffer);
}

void RenderComponentManager::createVertexBuffer(Buffer& VBO, Mesh& mesh) {
	size_t bufferSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();

	Buffer stagingBuffer;
	BufferManager::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);
	BufferManager::writeMem(stagingBuffer, mesh.vertices.data(), bufferSize);
	BufferManager::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VBO);
	BufferManager::copyBuffer(stagingBuffer, VBO, bufferSize);
	BufferManager::destroyBuffer(stagingBuffer);
}

void RenderComponentManager::createIndexBuffer(Buffer& IBO, Mesh& mesh) {
	size_t bufferSize = sizeof(mesh.indices[0]) * mesh.indices.size();

	Buffer stagingBuffer;
	BufferManager::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);
	BufferManager::writeMem(stagingBuffer, mesh.indices.data(), bufferSize);
	BufferManager::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IBO);
	BufferManager::copyBuffer(stagingBuffer, IBO, bufferSize);
	BufferManager::destroyBuffer(stagingBuffer);
}

void RenderComponentManager::destroyVertexBuffer(Buffer& VBO) {
	BufferManager::destroyBuffer(VBO);
}

void RenderComponentManager::destroyIndexBuffer(Buffer& IBO) {
	BufferManager::destroyBuffer(IBO);
}

void RenderComponentManager::createLightBuffer(Buffer& buffer) {
	BufferManager::createBuffer(
		(VkDeviceSize) lightBufferSize(),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		buffer
	);
}

void RenderComponentManager::createUniformBuffer(Buffer& UBO) {
	BufferManager::createBuffer(
		sizeof(UniformBufferObject),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		UBO
	);
}

void RenderComponentManager::createMaterialBuffer(Buffer& UBO) {
	BufferManager::createBuffer(
		sizeof(MaterialBuffer),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		UBO
	);
}

void RenderComponentManager::createBoneBuffer(Buffer& UBO, size_t numBones) {
	BufferManager::createBuffer(
		sizeof(glm::mat4) * numBones + 4 * sizeof(uint32_t),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		UBO
	);
}

void RenderComponentManager::updateBoneBuffer(RenderComponent& component, Buffer& boneBuffer) {
	const std::vector<glm::mat4>& boneMatrices = component.model->armature->getBoneMatrices();

	BufferManager::writeMemArray(
		boneBuffer,
		const_cast<glm::mat4 *>(boneMatrices.data()),
		(uint32_t)boneMatrices.size(),
		sizeof(glm::mat4)
	);
}

void RenderComponentManager::updateLightBuffer(Buffer& curBuffer) {
	BufferManager::writeMemArray(curBuffer, lights.data(), (uint32_t)lights.size(), sizeof(Light));
}

void RenderComponentManager::updateMaterialBuffer(RenderComponent& component, Buffer& curMatBuf) {
	MaterialBuffer mat;

	mat.baseColor = component.model->material->baseColor;
	mat.metallic = component.model->material->metallic;
	mat.specular = component.model->material->specular;
	mat.roughness = component.model->material->roughness;
	mat.emission = component.model->material->emission;
	mat.diffuseUVScale = component.model->material->diffuseUVScale;
	mat.normalUVScale = component.model->material->normalUVScale;
	mat.specularUVScale = component.model->material->specularUVScale;
	mat.ambientUVScale = component.model->material->ambientUVScale;
	mat.displaceUVScale = component.model->material->displaceUVScale;
	mat.sheen = component.model->material->sheen;
	mat.IOR = component.model->material->IOR;

	BufferManager::writeMem(curMatBuf, &mat, sizeof(mat));
}

void RenderComponentManager::updateUniformBuffer(UIComponent& component, Buffer& curUBO) {
	float sx = ((float)Swapchain::swapChainExtent.width) / Swapchain::swapChainExtent.height;
	float sy = ((float)Swapchain::swapChainExtent.height) / Swapchain::swapChainExtent.height;

	UniformBufferObject ubo = {
		.model = component.getFullGlobalMatrix(),
		.view = glm::mat4(1),
		.proj = glm::ortho<float>(-sx, sx, -sy, sy),
		.norm = glm::transpose(glm::inverse(ubo.model)),
		.color = component.color,	// TODO: Move to a 'Material' buffer
		.camPos = { 0, 0, 0},
		.uvScale = {1, 1}			// TODO: make uv scale per texture
	};

	// Correct Projection Matrix
	ubo.proj[1][1] *= -1;

	// Consider push constants
	BufferManager::writeMem(curUBO, &ubo, sizeof(ubo));
}

void RenderComponentManager::updateUniformBuffer(DebugVisualizerComponent& component, Buffer& curUBO) {

	UniformBufferObject ubo = {
		.model = component.globalTransform.getMatrix(),
		.view = CameraManager::getViewMatrix(),
		.proj = CameraManager::getProjectionMatrix(),
		.norm = glm::transpose(glm::inverse(ubo.model)),
		.color = component.color,	// TODO: Move to a 'Material' buffer
		.camPos = CameraManager::cam->transformComponent->getGlobalTransform().getPosition(),
		.uvScale = {1, 1}			// TODO: make uv scale per texture
	};

	// Correct Projection Matrix
	ubo.proj[1][1] *= -1;

	// Consider push constants
	BufferManager::writeMem(curUBO, &ubo, sizeof(ubo));
}

void RenderComponentManager::updateUniformBuffer(RenderComponent& component, Buffer& curUBO) {
	UniformBufferObject ubo = {
		.model = component.owner.transformComponent->getGlobalTransform().getMatrix(),
		.view = CameraManager::getViewMatrix(),
		.proj = glm::perspective(glm::radians(60.0f), Swapchain::swapChainExtent.width / (float)Swapchain::swapChainExtent.height, 0.01f, 1000.0f),
		.norm = glm::transpose(glm::inverse(ubo.model)),
		.color = { 1.0f, 1.0f, 1.0f, 1.0f },			// TODO : Move to a 'Material' buffer
		.camPos = CameraManager::cam->transformComponent->getGlobalTransform().getPosition(),
		.uvScale = {1, 1} // TODO: make uv scale per texture
	};

	// Correct Projection Matrix
	ubo.proj[1][1] *= -1;

	// Consider push constants
	BufferManager::writeMem(curUBO, &ubo, sizeof(ubo));
}

void RenderComponentManager::destroyLightBuffer(Buffer& buffer) {
	BufferManager::destroyBuffer(buffer);
}

void RenderComponentManager::destroyUniformBuffer(Buffer& UBO) {
	BufferManager::destroyBuffer(UBO);
}
void RenderComponentManager::destroyMaterialBuffer(Buffer& UBO) {
	BufferManager::destroyBuffer(UBO);
}
void RenderComponentManager::destroyBoneBuffer(Buffer& UBO) {
	BufferManager::destroyBuffer(UBO);
}


void RenderComponentManager::createUIDescriptorSets(Swapchain::Frame& frame, size_t componentIndex) {

	VkDescriptorImageInfo diffuseTextureInfo = {
		.sampler = uiComponents[componentIndex]->model->material->diffuseTex->sampler,
		.imageView = uiComponents[componentIndex]->model->material->diffuseTex->image->imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};


	VkDescriptorBufferInfo bufferInfo = {
		.buffer = frame.data.UI_UBOs[componentIndex].buffer,
		.offset = 0,
		.range = sizeof(UniformBufferObject)
	};

	VkWriteDescriptorSet descriptorWrites[] = {
		writeDescriptorSetBuffer(frame.data.UI_componentSpecificDescriptorSets[componentIndex], 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfo),
		writeDescriptorSetImage(frame.data.UI_componentSpecificDescriptorSets[componentIndex], 2, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, diffuseTextureInfo),
	};

	vkUpdateDescriptorSets(
		Device::logicalDevice,
		static_cast<uint32_t>(std::size(descriptorWrites)),
		descriptorWrites, 0, nullptr
	);
}

void RenderComponentManager::createDebugVisualizerDescriptorSets(Swapchain::Frame& frame, size_t componentIndex) {

	VkDescriptorImageInfo diffuseTextureInfo = {
		.sampler = debugVisualizerComponents[componentIndex]->model->material->diffuseTex->sampler,
		.imageView = debugVisualizerComponents[componentIndex]->model->material->diffuseTex->image->imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};


	VkDescriptorBufferInfo bufferInfo = {
		.buffer = frame.data.DebugVisualizer_UBOs[componentIndex].buffer,
		.offset = 0,
		.range = sizeof(UniformBufferObject)
	};

	VkWriteDescriptorSet descriptorWrites[] = {
		writeDescriptorSetBuffer(frame.data.DebugVisualizer_componentSpecificDescriptorSets[componentIndex], 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfo),
		writeDescriptorSetImage(frame.data.DebugVisualizer_componentSpecificDescriptorSets[componentIndex], 2, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, diffuseTextureInfo),
	};

	vkUpdateDescriptorSets(
		Device::logicalDevice,
		static_cast<uint32_t>(std::size(descriptorWrites)),
		descriptorWrites, 0, nullptr
	);
}

void RenderComponentManager::createDescriptorSets(Swapchain::Frame& frame, size_t componentIndex) {
	VkDescriptorImageInfo diffuseTextureInfo = {
		.sampler = components[componentIndex]->model->material->diffuseTex->sampler,
		.imageView = components[componentIndex]->model->material->diffuseTex->image->imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkDescriptorImageInfo normalTextureInfo = {
		.sampler = components[componentIndex]->model->material->normalTex->sampler,
		.imageView = components[componentIndex]->model->material->normalTex->image->imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};

	VkDescriptorImageInfo specularTextureInfo = {
		.sampler = components[componentIndex]->model->material->specularTex->sampler,
		.imageView = components[componentIndex]->model->material->specularTex->image->imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};

	VkDescriptorImageInfo ambientTextureInfo = {
		.sampler = components[componentIndex]->model->material->ambientTex->sampler,
		.imageView = components[componentIndex]->model->material->ambientTex->image->imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};

	VkDescriptorImageInfo displaceTextureInfo = {
		.sampler = components[componentIndex]->model->material->displaceTex->sampler,
		.imageView = components[componentIndex]->model->material->displaceTex->image->imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};

	VkDescriptorBufferInfo bufferInfo = {
		.buffer = frame.data.UBOs[componentIndex].buffer,
		.offset = 0,
		.range = sizeof(UniformBufferObject)
	};

	VkDescriptorBufferInfo bufferInfo2 = {
		.buffer = frame.data.MatBuffers[componentIndex].buffer,
		.offset = 0,
		.range = sizeof(MaterialBuffer)
	};

	VkDescriptorBufferInfo boneBufferInfo = {
		.buffer = frame.data.BoneBuffers[componentIndex].buffer,
		.offset = 0,
		.range = sizeof(glm::mat4) * components[componentIndex]->model->armature->getBoneMatrices().size() + 4 * sizeof(uint32_t)
	};

	VkWriteDescriptorSet descriptorWrites[] = {
		writeDescriptorSetBuffer(frame.data.componentSpecificDescriptorSets[componentIndex], 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfo),
		writeDescriptorSetBuffer(frame.data.componentSpecificDescriptorSets[componentIndex], 4, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfo2),
		writeDescriptorSetBuffer(frame.data.componentSpecificDescriptorSets[componentIndex], 8, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, boneBufferInfo),
		writeDescriptorSetImage(frame.data.componentSpecificDescriptorSets[componentIndex], 2, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, diffuseTextureInfo),
		writeDescriptorSetImage(frame.data.componentSpecificDescriptorSets[componentIndex], 3, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, normalTextureInfo),
		writeDescriptorSetImage(frame.data.componentSpecificDescriptorSets[componentIndex], 5, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, specularTextureInfo),
		writeDescriptorSetImage(frame.data.componentSpecificDescriptorSets[componentIndex], 6, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ambientTextureInfo),
		writeDescriptorSetImage(frame.data.componentSpecificDescriptorSets[componentIndex], 7, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, displaceTextureInfo)
	};

	vkUpdateDescriptorSets(
		Device::logicalDevice,
		static_cast<uint32_t>(std::size(descriptorWrites)),
		descriptorWrites, 0, nullptr
	);
}