#pragma once
#include <iostream>
#include <string>

class DebugManager {
public:
	enum class Priority { LOG_PRIORITY, WARNING_PRIORITY, ERROR_PRIORITY };

	static unsigned indentLevel;

	static void message(std::string msg, Priority priority = Priority::LOG_PRIORITY);
};