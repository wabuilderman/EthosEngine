#pragma once

class Instance;
class Window;

class Engine {
public:
	static Window* window;
	static Instance* instance;

	static void init();
	static void update();
	static void shutdown();

	static void rest();
	static bool shouldClose();
};