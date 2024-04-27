#pragma once
#include "pch.hpp"

#include <vector>
#include "DebugMessenger.hpp"

#include "Engine.hpp"


class Surface;

const static std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
};

class Instance {
	static bool checkValidationLayerSupport();
	static std::vector<const char*> getRequiredExtensions();

	
	DebugMessenger* debugMessenger;
	VkInstance instance;
	
public:
	Surface* surface;

	operator const VkInstance&() const { return instance; }
	operator VkInstance&() { return instance; }

	void drawFrame();

	Instance();
	~Instance();
};