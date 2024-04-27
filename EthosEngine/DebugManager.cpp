#include "DebugManager.hpp"
unsigned DebugManager::indentLevel = 0;

void DebugManager::message(std::string msg, Priority priority) {

	for (unsigned i = 0; i < indentLevel; ++i) {
		std::cout << "  ";
	}

	switch (priority) {
	case Priority::LOG_PRIORITY:
		std::cout << "LOG: ";
		break;

	case Priority::WARNING_PRIORITY:
		std::cout << "WARNING: ";
		break;

	case Priority::ERROR_PRIORITY:
		std::cout << "ERROR: ";
		break;
	}
	std::cout << msg << std::endl;
}