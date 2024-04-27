#pragma once
#include "pch.hpp"
#include "Mesh.hpp"
#include "DebugManager.hpp"
#include "Transform.hpp"

class RectMesh : public Mesh {
public:
	static RectMesh* get() {
		static RectMesh mesh = RectMesh();
		return &mesh;
	}

	RectMesh() {
		vertices.push_back({
			.pos = Vector::zero,
			.normal = Vector::forward,
			.tangent = Vector::up,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {0.0f, 0.0f}
		});
		vertices.push_back({
			.pos = Vector::right,
			.normal = Vector::forward,
			.tangent = Vector::up,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {0.0f, 1.0f}
		});
		vertices.push_back({
			.pos = Vector::up,
			.normal = Vector::forward,
			.tangent = Vector::up,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {1.0f, 0.0f}
		});
		vertices.push_back({
			.pos = Vector::up + Vector::right,
			.normal = Vector::forward,
			.tangent = Vector::up,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {1.0f, 1.0f}
		});

		indices = {
			0, 2, 1,
			3, 1, 2
		};
	}
};