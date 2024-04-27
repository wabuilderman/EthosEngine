#include "GraphicsPipeline.hpp"

#include "Engine.hpp"
#include "Instance.hpp"
#include "Surface.hpp"
#include "WindowWrapper.hpp"

#include "engineUtil.hpp"
#include "Swapchain.hpp"
#include "SyncManager.hpp"
#include "Vertex.hpp"
#include "BufferManager.hpp"
#include "CommandPool.hpp"

#include "DescriptorManager.hpp"
#include "RenderComponentManager.hpp"
#include "RenderComponent.hpp"

#include "Shader.hpp"

void GraphicsPipeline::createCommandPool() {
	// Retrieve Queue Family Indices
	QueueFamilies::QueueFamilyIndices queueFamilyIndices =
		QueueFamilies::findQueueFamilies(Device::physicalDevice, *Engine::instance->surface);

	// Specify Command Pool Settings
	VkCommandPoolCreateInfo poolInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = 0,
		.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
	};

	// Create Command Pool
	if (vkCreateCommandPool(Device::logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void GraphicsPipeline::rerecordCommandBuffers() {
	vkFreeCommandBuffers(Device::logicalDevice, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	createCommandBuffers();
}

void GraphicsPipeline::rerecordCommandBuffer(size_t index) {
	vkFreeCommandBuffers(Device::logicalDevice, commandPool, 1, &commandBuffers[index]);
	createCommandBuffer(index);
}

void GraphicsPipeline::createCommandBuffer(size_t index) {
	// Specify Command Buffer Allocation Settings
	VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	// Allocate Command Buffers
	if (vkAllocateCommandBuffers(swapchain.device, &allocInfo, &commandBuffers[index]) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
	
	recordCommandBuffer(index);
}

void GraphicsPipeline::recordCommandBuffer(size_t index) {
	// Specify Additional Settings
	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = 0, // Optional
		.pInheritanceInfo = nullptr // Optional
	};

	// Begin Recording Command Buffer
	if (vkBeginCommandBuffer(commandBuffers[index], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	// Specify Clear Values
	VkClearValue clearValues[] = {
		{.color = { 0.1f, 0.1f, 0.1f, 1.0f }},
		{.depthStencil = { 1.0f, 0 }}
	};

	// Specify Render Pass Command
	VkRenderPassBeginInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = renderPass,
		.framebuffer = Swapchain::frames[index].framebuffer,
		.renderArea = {.offset = {.x = 0, .y = 0 }, .extent = swapchain.swapChainExtent },
		.clearValueCount = (uint32_t)std::size(clearValues),
		.pClearValues = clearValues
	};

	// Begin Render Pass
	vkCmdBeginRenderPass(commandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind Graphics Pipeline
	vkCmdBindPipeline(commandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[0]);

	if (Swapchain::metaFrameData.frameSpecificDescriptorSets.size() != 0) {
		// Bind Descriptor Sets
		vkCmdBindDescriptorSets(commandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1,
			&(Swapchain::metaFrameData.frameSpecificDescriptorSets[index]), 0, nullptr);
	}

	// For Each Render Component
	for (size_t j = 0; j < RenderComponentManager::components.size(); ++j) {
		if (!RenderComponentManager::components[j]->isVisible)
			continue;
		if (!RenderComponentManager::components[j]->model->material->isOpaque) continue;
		size_t mesh_index = RenderComponentManager::getMeshIndex(RenderComponentManager::components[j]->model->mesh);

		VkBuffer vertexBuffers[] = { RenderComponentManager::VBOs[mesh_index].buffer };
		VkDeviceSize offsets[] = { 0 };

		// Bind Vertices
		vkCmdBindVertexBuffers(commandBuffers[index], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[index], RenderComponentManager::IBOs[mesh_index].buffer, 0, VK_INDEX_TYPE_UINT32);

		// Bind Descriptor Sets
		vkCmdBindDescriptorSets(commandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout, 0, 1, &(Swapchain::frames[index].data.componentSpecificDescriptorSets[j]), 0, nullptr);

		// Draw Vertices
		vkCmdDrawIndexed(commandBuffers[index],
			static_cast<uint32_t>(RenderComponentManager::components[j]->model->mesh->indices.size()),
			1, 0, 0, 0);
	}

	vkCmdNextSubpass(commandBuffers[index], VK_SUBPASS_CONTENTS_INLINE);

	// Bind Graphics Pipeline
	vkCmdBindPipeline(commandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[1]);

	for (size_t j = 0; j < RenderComponentManager::uiComponents.size(); ++j) {
		if (!RenderComponentManager::uiComponents[j]->isVisible)
			continue;

		size_t mesh_index = RenderComponentManager::getMeshIndex(RenderComponentManager::uiComponents[j]->model->mesh);

		VkBuffer vertexBuffers[] = { RenderComponentManager::VBOs[mesh_index].buffer };
		VkDeviceSize offsets[] = { 0 };

		// Bind Vertices
		vkCmdBindVertexBuffers(commandBuffers[index], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[index],
			RenderComponentManager::IBOs[mesh_index].buffer, 0, VK_INDEX_TYPE_UINT32);

		// Bind Descriptor Sets
		vkCmdBindDescriptorSets(commandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
			&(Swapchain::frames[index].data.UI_componentSpecificDescriptorSets[j]), 0, nullptr);

		// Draw Vertices
		vkCmdDrawIndexed(commandBuffers[index], static_cast<uint32_t>(RenderComponentManager::uiComponents[j]->model->mesh->indices.size()), 1, 0, 0, 0);
	}

	vkCmdNextSubpass(commandBuffers[index], VK_SUBPASS_CONTENTS_INLINE);

	// Bind Graphics Pipeline
	vkCmdBindPipeline(commandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[2]);

	for (size_t j = 0; j < RenderComponentManager::debugVisualizerComponents.size(); ++j) {
		if (!RenderComponentManager::debugVisualizerComponents[j]->isVisible)
			continue;

		size_t mesh_index = RenderComponentManager::getMeshIndex(RenderComponentManager::debugVisualizerComponents[j]->model->mesh);

		VkBuffer vertexBuffers[] = { RenderComponentManager::VBOs[mesh_index].buffer };
		VkDeviceSize offsets[] = { 0 };

		// Bind Vertices
		vkCmdBindVertexBuffers(commandBuffers[index], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[index],
			RenderComponentManager::IBOs[mesh_index].buffer, 0, VK_INDEX_TYPE_UINT32);

		// Bind Descriptor Sets
		vkCmdBindDescriptorSets(commandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
			&(Swapchain::frames[index].data.DebugVisualizer_componentSpecificDescriptorSets[j]), 0, nullptr);

		// Draw Vertices
		vkCmdDrawIndexed(commandBuffers[index], static_cast<uint32_t>(RenderComponentManager::debugVisualizerComponents[j]->model->mesh->indices.size()), 1, 0, 0, 0);
	}

	// End Render Pass
	vkCmdEndRenderPass(commandBuffers[index]);

	// End Recording Command Buffer
	if (vkEndCommandBuffer(commandBuffers[index]) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void GraphicsPipeline::createCommandBuffers() {
	// Resize to match number of frames
	commandBuffers.resize(Swapchain::frames.size());

	// Specify Command Buffer Allocation Settings
	VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = (uint32_t)commandBuffers.size()
	};

	// Allocate Command Buffers
	if (vkAllocateCommandBuffers(swapchain.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	// For Each Command Buffer
	for (size_t i = 0; i < commandBuffers.size(); i++) {
		recordCommandBuffer(i);
	}
}

void GraphicsPipeline::createFramebuffers() {
	// Create a framebuffer for each frame
	for (size_t i = 0; i < Swapchain::frames.size(); ++i) {
		// Specify Attachments
		VkImageView attachments[] = {
			Swapchain::frames[i].imageView, // Color Attachment Image View
			depthImage->imageView           // Depth Attachment Image View
		};

		// Specify Framebuffer Settings
		VkFramebufferCreateInfo framebufferInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = (uint32_t)std::size(attachments),
			.pAttachments = attachments,
			.width = swapchain.swapChainExtent.width,
			.height = swapchain.swapChainExtent.height,
			.layers = 1
		};

		// Create Framebuffer
		if (vkCreateFramebuffer(swapchain.device, &framebufferInfo, nullptr, &Swapchain::frames[i].framebuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void GraphicsPipeline::createRenderPass() {
	VkAttachmentDescription attachments[] = {
		{ // Color attachment
			.format = swapchain.swapChainImageFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		},{ // Depth Attachment
			.format = (VkFormat)findDepthFormat(),
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		}
	};

	VkAttachmentReference colorAttachmentRef = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkAttachmentReference depthAttachmentRef = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpasses[] = {{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
		.pDepthStencilAttachment = &depthAttachmentRef
		},{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
		.pDepthStencilAttachment = nullptr
		},{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
		.pDepthStencilAttachment = &depthAttachmentRef
	}};

	// Specify Subpass Dependencies
	VkSubpassDependency dependencies[] = {{
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 1,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		},{
		.srcSubpass = 0,
		.dstSubpass = 1,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		},{
		.srcSubpass = 0,
		.dstSubpass = VK_SUBPASS_EXTERNAL,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
	}};

	VkRenderPassCreateInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = (uint32_t) std::size(attachments),
		.pAttachments = attachments,
		.subpassCount = (uint32_t) std::size(subpasses),
		.pSubpasses = subpasses,
		.dependencyCount = (uint32_t) std::size(dependencies),
		.pDependencies = dependencies
	};
	

	// Create Render Pass
	if (vkCreateRenderPass(swapchain.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void GraphicsPipeline::createGraphicsPipelineLayout() {
	// Specify Pipeline Layout CreateInfo

	VkDescriptorSetLayout layouts[] = {
		DescriptorManager::componentSpecificDescriptorSetLayout,
		DescriptorManager::frameSpecificDescriptorSetLayout
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = (uint32_t) std::size(layouts),
		.pSetLayouts = layouts,
		.pushConstantRangeCount = 0, // Optional
		.pPushConstantRanges = nullptr // Optional
	};

	// Create the Pipeline Layout
	if (vkCreatePipelineLayout(swapchain.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void GraphicsPipeline::createGraphicsPipeline() {
	// Load Shaders from file
	Shader vertexShader("shaders/shader.vert.spv", "main", VK_SHADER_STAGE_VERTEX_BIT);
	Shader fragmentShader("shaders/shader.frag.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT);

	Shader UI_vertexShader("shaders/ui.vert.spv", "main", VK_SHADER_STAGE_VERTEX_BIT);
	Shader UI_fragmentShader("shaders/ui.frag.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT);

	Shader debugVisualizer_vertexShader("shaders/debug.vert.spv", "main", VK_SHADER_STAGE_VERTEX_BIT);
	Shader debugVisualizer_fragmentShader("shaders/debug.frag.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT);

	// List of the stages
	VkPipelineShaderStageCreateInfo primaryShaderStages[] = {
		vertexShader.getStageCreateInfo(),
		fragmentShader.getStageCreateInfo()
	};

	VkPipelineShaderStageCreateInfo uiShaderStages[] = {
		UI_vertexShader.getStageCreateInfo(),
		UI_fragmentShader.getStageCreateInfo()
	};

	VkPipelineShaderStageCreateInfo debugVisualizerShaderStages[] = {
		debugVisualizer_vertexShader.getStageCreateInfo(),
		debugVisualizer_fragmentShader.getStageCreateInfo()
	};

	// Specify Vertex Bindings and Attribute Descriptions
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &Vertex::getBindingDescription(),
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::getAttributeDescriptions().size()),
		.pVertexAttributeDescriptions = Vertex::getAttributeDescriptions().data()
	};

	// Specify Topolgy
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	// Specify Viewport Dimensions
	VkViewport viewport = {
		.x = 0.0f, .y = 0.0f,
		.width = (float) Swapchain::swapChainExtent.width,
		.height = (float) Swapchain::swapChainExtent.height,
		.minDepth = 0.0f, .maxDepth = 1.0f
	};

	// Specify Clipping Mask
	VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = swapchain.swapChainExtent
	};

	// Combine Viewport & Scissor
	VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor
	};

	// Specify how rasterization should be handled
	VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL, // VK_POLYGON_MODE_LINE;
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f, // Optional
		.depthBiasClamp = 0.0f, // Optional
		.depthBiasSlopeFactor = 0.0f, // Optional
		.lineWidth = 1.0f,
	};

	// Specify how multisampling should be handled
	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f, // Optional
		.pSampleMask = nullptr, // Optional
		.alphaToCoverageEnable = VK_FALSE, // Optional
		.alphaToOneEnable = VK_FALSE // Optional
	};

	// Specify how colorblending should be handled
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	// Further Specify Colorblending
	VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY, // Optional
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants = {0.0f, 0.0f, 0.0f, 0.0f} // Optional
	};

	VkPipelineDepthStencilStateCreateInfo depthStencil = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front = {}, // Optional
		.back = {}, // Optional
		.minDepthBounds = 0.0f, // Optional
		.maxDepthBounds = 1.0f // Optional
	};

	// Specify Graphics Pipeline Components
	VkGraphicsPipelineCreateInfo pipelineInfo[] = {
		{ // Primary Subpass
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = (uint32_t)std::size(primaryShaderStages),
			.pStages = primaryShaderStages,
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pDepthStencilState = &depthStencil,
			.pColorBlendState = &colorBlending,
			.pDynamicState = nullptr,
			.layout = pipelineLayout,
			.renderPass = renderPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,	// Optional
			.basePipelineIndex = -1,				// Optional
		},
		{ // UI Subpass
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = (uint32_t)std::size(uiShaderStages),
			.pStages = uiShaderStages,
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pDepthStencilState = &depthStencil,
			.pColorBlendState = &colorBlending,
			.pDynamicState = nullptr,
			.layout = pipelineLayout,
			.renderPass = renderPass,
			.subpass = 1,
			.basePipelineHandle = VK_NULL_HANDLE,	// Optional
			.basePipelineIndex = -1					// Optional
		}
		,
		{ // Debug Subpass
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = (uint32_t)std::size(debugVisualizerShaderStages),
			.pStages = debugVisualizerShaderStages,
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pDepthStencilState = &depthStencil,
			.pColorBlendState = &colorBlending,
			.pDynamicState = nullptr,
			.layout = pipelineLayout,
			.renderPass = renderPass,
			.subpass = 2,
			.basePipelineHandle = VK_NULL_HANDLE,	// Optional
			.basePipelineIndex = -1					// Optional
		}
	};

	// TODO: Investigate 'pipelineCache'
	// Create Graphics Pipelines
	if (vkCreateGraphicsPipelines(Device::logicalDevice, VK_NULL_HANDLE, (uint32_t)std::size(pipelineInfo), pipelineInfo, nullptr, graphicsPipelines.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void GraphicsPipeline::drawFrame() {
	vkWaitForFences(swapchain.device, 1, &swapchain.syncManager->inFlightFences[swapchain.syncManager->currentFrame], VK_TRUE, UINT64_MAX);

#pragma warning(push)
#pragma warning(disable: 26812)
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(
		swapchain.device, swapchain, UINT64_MAX,
		swapchain.syncManager->imageAvailableSemaphores[swapchain.syncManager->currentFrame],
		VK_NULL_HANDLE, &imageIndex
	);
#pragma warning(pop)

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		swapchain.recreate();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	swapchain.syncManager->checkFrame(imageIndex);

	// Update Uniform Buffers
	RenderComponentManager::updateAllUniformBuffers(imageIndex);

	VkSemaphore waitSemaphores[] = { swapchain.syncManager->currentImageAvailableSemaphore() };
	VkSemaphore signalSemaphores[] = { swapchain.syncManager->renderFinishedSemaphores[swapchain.syncManager->currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffers[imageIndex],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores
	};

	vkResetFences(Device::logicalDevice, 1, &(swapchain.syncManager->inFlightFences[swapchain.syncManager->currentFrame]));

	if (vkQueueSubmit(swapchain.device.graphicsQueue, 1, &submitInfo, swapchain.syncManager->currentInFlightFence()) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkSwapchainKHR swapChains[] = { swapchain.swapchain };

	VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = (uint32_t)std::size(signalSemaphores),
		.pWaitSemaphores = signalSemaphores,
		.swapchainCount = (uint32_t)std::size(swapChains),
		.pSwapchains = swapChains,
		.pImageIndices = &imageIndex,
		.pResults = nullptr // Optional
	};

	result = vkQueuePresentKHR(swapchain.device.presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || Engine::window->resized) {
		swapchain.recreate();
		Engine::window->resized = false;
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	swapchain.syncManager->incrementFrame();
}

void GraphicsPipeline::init() {
	createRenderPass();
	createGraphicsPipeline();

	createDepthResources();
	createFramebuffers();

	//DescriptorManager::createDescriptorPool();
	//RenderComponentManager::recreateAllDescriptorSets();

	createCommandBuffers();
}

void GraphicsPipeline::cleanup() {
	for (auto frame : Swapchain::frames) {
		vkDestroyFramebuffer(swapchain.device, frame.framebuffer, nullptr);
	}

	delete depthImage;

	vkFreeCommandBuffers(swapchain.device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	//DescriptorManager::destroyDescriptorPool();

	vkDestroyPipeline(swapchain.device, graphicsPipelines[0], nullptr);
	vkDestroyPipeline(swapchain.device, graphicsPipelines[1], nullptr);
	vkDestroyRenderPass(swapchain.device, renderPass, nullptr);
}

void GraphicsPipeline::recreate() {
	cleanup();
	init();
	DebugManager::message("Graphics Pipeline Recreated");
}

#pragma warning(push)
#pragma warning(disable: 26812)
VkFormat GraphicsPipeline::findDepthFormat() {
	return swapchain.device.findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

void GraphicsPipeline::createDepthResources() {
	uint32_t height = swapchain.swapChainExtent.height;
	uint32_t width = swapchain.swapChainExtent.width;
	VkFormat depthFormat = findDepthFormat();
	VkImageTiling tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	VkMemoryPropertyFlags memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	depthImage = new ImageResource(width, height, 1, depthFormat, tiling, usage, memProperties, aspectMask);

	depthImage->transitionImageLayout(depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}
#pragma warning(pop)

GraphicsPipeline::GraphicsPipeline(Swapchain& swapchain) : swapchain(swapchain) {
	DescriptorManager::createComponentSpecificDescriptorSetLayout();
	DescriptorManager::createFrameSpecificDescriptorSetLayout();
	createGraphicsPipelineLayout();
	createCommandPool();

	RenderComponentManager::init(this);

	init();
	DebugManager::message("Graphics Pipeline Created");
}

GraphicsPipeline::~GraphicsPipeline() {
	cleanup();

	RenderComponentManager::cleanup();

	vkDestroyPipelineLayout(swapchain.device, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(swapchain.device, DescriptorManager::frameSpecificDescriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(swapchain.device, DescriptorManager::componentSpecificDescriptorSetLayout, nullptr);

	vkDestroyCommandPool(swapchain.device, commandPool, nullptr);

	DebugManager::message("Graphics Pipeline Destroyed");
}