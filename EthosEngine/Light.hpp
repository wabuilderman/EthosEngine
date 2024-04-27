#pragma once
#include "pch.hpp"

#include <array>

struct Light {
	enum class Type : uint32_t { Point, Sun, Spot, Area };

	alignas(16) glm::vec3 pos = { 0, 0, 0 };
	alignas(16) glm::vec4 color = { 1, 1, 1, 1 };
	alignas(16) glm::vec3 dir = { 0, 0, 0 };

	alignas(4) Type type = Type::Point;
};