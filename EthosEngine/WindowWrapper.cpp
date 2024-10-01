#include "WindowWrapper.hpp"

#include "engineUtil.hpp"
#include "Instance.hpp"


uint32_t Window::width = 800;
uint32_t Window::height = 600;

Window::Window() {
	printf("Initializing Window...");
	if (!glfwInit()) {
		printf("Critical Error - failed to intialize GLFW");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, resizeCallback);

	DebugManager::message("Window Created");
}

void Window::resizeCallback(GLFWwindow* window, int width, int height) {
	auto self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	Window::width = width;
	Window::height = height;
	self->resized = true;
}

void Window::setWindowName(const char* name) {
	glfwSetWindowTitle(window, name);
}

void Window::pollEvents() {
	glfwPollEvents();
}

bool Window::shouldClose() {
	return glfwWindowShouldClose(window);
}

Window::~Window() {
	glfwDestroyWindow(window);
	glfwTerminate();
	DebugManager::message("Window Destroyed");
}