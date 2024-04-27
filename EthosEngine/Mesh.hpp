#pragma once
#include <vector>
#include <string>
#include "Vertex.hpp"

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	void loadOBJ(const std::string& path);

	Mesh(const std::string& path);
	Mesh();
};
