#pragma once
#include "pch.hpp"

class DebugMessenger {
	VkDebugUtilsMessengerEXT obj;
	VkInstance& inst;

public:
	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	DebugMessenger(VkInstance& instance);
	~DebugMessenger();
};