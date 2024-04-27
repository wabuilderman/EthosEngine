#pragma once
#include "pch.hpp"

#include "Message.hpp"

class Window {
	GLFWwindow* window;
public:

	static uint32_t width;
	static uint32_t height;

	bool resized = false;

	operator GLFWwindow*&() { return window; }

	static void resizeCallback(GLFWwindow* window, int width, int height);
	void setWindowName(const char* name);

	void pollEvents();
	bool shouldClose();

	Window();
	~Window();
};