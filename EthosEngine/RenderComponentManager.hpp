#pragma once

#include "pch.hpp"

#include <vector>

#include "RenderComponent.hpp"
#include "UIComponent.hpp"
#include "DebugVisualizerComponent.hpp"
#include "Model.hpp"
#include "Texture.hpp"
#include "BufferManager.hpp"

#include "GraphicsPipeline.hpp"

class RenderComponentManager {
	static void createDescriptorSets(Swapchain::Frame& frame, size_t componentIndex);
	static void createUIDescriptorSets(Swapchain::Frame& frame, size_t componentIndex);
	static void createDebugVisualizerDescriptorSets(Swapchain::Frame& frame, size_t componentIndex);

	static void createVertexBuffer(Buffer& VBO, Mesh& mesh);
	static void updateVertexBuffer(Buffer& VBO, Mesh& mesh);
	static void destroyVertexBuffer(Buffer& VBO);

	static void createIndexBuffer(Buffer& IBO, Mesh& mesh);
	static void destroyIndexBuffer(Buffer& IBO);

	static void createUniformBuffer(Buffer& UBO);
	static void createMaterialBuffer(Buffer& UBO);
	static void createBoneBuffer(Buffer& UBO, size_t numBones);
	static void createLightBuffer(Buffer& buffer);

	static void updateMaterialBuffer(RenderComponent& component, Buffer& curMatBuf);
	static void updateBoneBuffer(RenderComponent& component, Buffer& boneBuffer);
	static void updateLightBuffer(Buffer& curBuffer);

	static void updateUniformBuffer(RenderComponent& component, Buffer& curUBO);
	static void updateUniformBuffer(UIComponent& component, Buffer& curUBO);
	static void updateUniformBuffer(DebugVisualizerComponent& component, Buffer& curUBO);

	static void destroyUniformBuffer(Buffer& UBO);
	static void destroyMaterialBuffer(Buffer& UBO);
	static void destroyBoneBuffer(Buffer& UBO);
	static void destroyLightBuffer(Buffer& buffer);
	static GraphicsPipeline* graphicsPipeline;

public:
	
	typedef size_t LightID;

	static std::vector<RenderComponent*> components;
	static std::vector<UIComponent*> uiComponents;
	static std::vector<DebugVisualizerComponent*> debugVisualizerComponents;
	static std::vector<Buffer> VBOs;
	static std::vector<Buffer> IBOs;


	static void init(GraphicsPipeline* graphicsPipeline);
	static void cleanup();

	static void addMesh(Mesh* mesh);
	static void updateMesh(Mesh* mesh);
	static void removeMesh(Mesh* mesh);
	static size_t getMeshIndex(Mesh* mesh);

	static void addUIComponent(UIComponent* component);
	static void updateUIComponentVBO(UIComponent* component);
	static void removeUIComponent(UIComponent* component);

	static void addDebugVisualizerComponent(DebugVisualizerComponent* component);
	static void updateDebugVisualizerComponentVBO(DebugVisualizerComponent* component);
	static void removeDebugVisualizerComponent(DebugVisualizerComponent* component);

	static void addRenderComponent(RenderComponent* component);
	static void removeRenderComponent(RenderComponent* component);
	static void updateRenderComponentVisibility(RenderComponent* component, bool visible);

	static LightID addLight(const Light& light);
	static Light* getLight(LightID lightID);
	static void removeLight(LightID light);

	static void recreate();
	static void updateAllUniformBuffers(uint32_t currentImage);

	static void zSortComponents(uint32_t currentImage);
};