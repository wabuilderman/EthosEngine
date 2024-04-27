#pragma once
#include "pch.hpp"
#include "Mesh.hpp"
#include "Transform.hpp"
#include <map>

class CubeMesh : public Mesh {
private:
	static constexpr unsigned numSubdivisions = 2;

	using Index = uint32_t;
	using Lookup = std::map<std::pair<Index, Index>, Index>;

	void addRect(const glm::vec3& norm, const glm::vec3& tan) {

		const glm::vec3 cot = glm::cross(norm, tan);

		vertices.push_back({
			.pos = norm * 0.5f + cot * -0.5f + tan * -0.5f,
			.normal = norm,
			.tangent = tan,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {0.0f, 0.0f}
			});

		vertices.push_back({
			.pos = norm * 0.5f + cot * 0.5f + tan * -0.5f,
			.normal = norm,
			.tangent = tan,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {0.0f, 1.0f}
			});

		vertices.push_back({
			.pos = norm * 0.5f + cot * -0.5f + tan * 0.5f,
			.normal = norm,
			.tangent = tan,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {1.0f, 0.0f}
			});

		vertices.push_back({
			.pos = norm * 0.5f + cot * 0.5f + tan * 0.5f,
			.normal = norm,
			.tangent = tan,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {1.0f, 1.0f}
			});

		/*unsigned int i = indices.size();
		indices.push_back(i);
		indices.push_back(i + 2);
		indices.push_back(i + 1);
		indices.push_back(i + 3);
		indices.push_back(i + 1);
		indices.push_back(i + 2);*/
	}

public:
	static CubeMesh* get() {
		static CubeMesh mesh = CubeMesh();
		return &mesh;
	}

	CubeMesh() {
		
		addRect(Vector::up, Vector::forward);
		addRect(Vector::down, Vector::backward);
		addRect(Vector::forward, Vector::down);
		addRect(Vector::backward, Vector::up);
		addRect(Vector::left, Vector::forward);
		addRect(Vector::right, Vector::backward);

		indices = {
			0, 2, 1, 3, 1, 2,
			4, 6, 5, 7, 5, 6,
			8, 10, 9, 11, 9, 10,
			12, 14, 13, 15, 13, 14,
			16, 18, 17, 19, 17, 18,
			20, 22, 21, 23, 21, 22
		};
	}
};