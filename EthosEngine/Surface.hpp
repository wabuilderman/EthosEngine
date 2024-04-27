#pragma once
#include "pch.hpp"
#include "Instance.hpp"

class Device;

class Surface {
public:
	static VkSurfaceKHR surface;
	Device* device;

	operator const VkSurfaceKHR&() const { return surface; }
	operator VkSurfaceKHR&() { return surface; }

	void drawFrame();

	Surface();
	~Surface();
};