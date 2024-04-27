#pragma once
#include "Mesh.hpp"
#include "DebugManager.hpp"
#include "Transform.hpp"

class AxisMesh : public Mesh {
private:
	static constexpr glm::vec3 origin = { 0.0f, 0.0f, 0.0f };

	static constexpr glm::vec3 red = { 1.0f, 0.0f , 0.0f };
	static constexpr glm::vec3 green = { 0.0f, 1.0f , 0.0f };
	static constexpr glm::vec3 blue = { 0.0f, 0.0f , 1.0f };

public:

	static AxisMesh* get() {
		static AxisMesh mesh = AxisMesh();
		return &mesh;
	}

	static AxisMesh* getMicro() {
		static AxisMesh mesh = AxisMesh();
		for (Vertex& vert : mesh.vertices) {
			vert.pos *= 0.05f;
		}
		return &mesh;
	}

	AxisMesh() {
		Vector x_end = origin + Vector::forward;
		Vector y_end = origin + Vector::right;
		Vector z_end = origin + Vector::up;

		glm::vec4 x_color = glm::vec4(red, 1.0f);
		glm::vec4 y_color = glm::vec4(green, 1.0f);
		glm::vec4 z_color = glm::vec4(blue, 1.0f);
		
		glm::vec2 uv = { 0.0f, 0.0f };

		float thickness = 0.05f;

		glm::vec3 v0 = origin + Vector::right * thickness;
		glm::vec3 v1 = origin + Vector::up    * thickness;
		glm::vec3 v2 = origin - Vector::right * thickness;
		glm::vec3 v3 = origin - Vector::up    * thickness;
		glm::vec3 v4 = x_end + Vector::right * thickness;
		glm::vec3 v5 = x_end + Vector::up    * thickness;
		glm::vec3 v6 = x_end - Vector::right * thickness;
		glm::vec3 v7 = x_end - Vector::up    * thickness;

		glm::vec3 n0 = Vector::right;
		glm::vec3 n1 = Vector::up;
		glm::vec3 n2 = Vector::left;
		glm::vec3 n3 = Vector::down;
		glm::vec3 n4 = glm::normalize(Vector::right + Vector::forward);
		glm::vec3 n5 = glm::normalize(Vector::up    + Vector::forward);
		glm::vec3 n6 = glm::normalize(Vector::left  + Vector::forward);
		glm::vec3 n7 = glm::normalize(Vector::down  + Vector::forward);

		glm::vec3 t0 = Vector::up;
		glm::vec3 t1 = Vector::left;
		glm::vec3 t2 = Vector::down;
		glm::vec3 t3 = Vector::right;
		glm::vec3 t4 = Vector::up;
		glm::vec3 t5 = Vector::left;
		glm::vec3 t6 = Vector::down;
		glm::vec3 t7 = Vector::right;

		// make axis (hollow cube)
		vertices.push_back({v0, n0, t0, x_color, uv });
		vertices.push_back({v1,n1,t1, x_color, uv });

		vertices.push_back({v2,n2,t2, x_color, uv });
		vertices.push_back({v3,n3,t3, x_color, uv });

		vertices.push_back({v4,n4,t4, x_color, uv });
		vertices.push_back({v5,n5,t5, x_color, uv });

		vertices.push_back({v6,n6,t6, x_color, uv });
		vertices.push_back({v7,n7,t7, x_color, uv });

		indices = {
			0, 1, 4,
			1, 5, 4,
			1, 2, 5,
			2, 6, 5,
			2, 3, 6,
			3, 7, 6,
			3, 0, 7,
			0, 4, 7,
			4, 5, 6,
			4, 6, 7
		};

		unsigned numTri = 10;

		for (unsigned i = 0; i < 8; ++i) {
			Vertex v = vertices[i];
			v.color = y_color;
			v.normal = { v.normal.y, v.normal.x, v.normal.z };
			v.pos = { v.pos.y, v.pos.x, v.pos.z };
			vertices.push_back(v);
		}

		for (unsigned i = 0; i < 8; ++i) {
			Vertex v = vertices[i];
			v.color = z_color;
			v.normal = { v.normal.z, v.normal.y, v.normal.x };
			v.pos = { v.pos.z, v.pos.y, v.pos.x };
			vertices.push_back(v);
		}

		for (unsigned i = 0; i < numTri; ++i) {
			indices.push_back(indices[(size_t)i * 3 + 0] + 8);
			indices.push_back(indices[(size_t)i * 3 + 2] + 8);
			indices.push_back(indices[(size_t)i * 3 + 1] + 8);
		}

		for (unsigned i = 0; i < numTri; ++i) {
			indices.push_back(indices[(size_t)i * 3 + 0] + 8*2);
			indices.push_back(indices[(size_t)i * 3 + 2] + 8*2);
			indices.push_back(indices[(size_t)i * 3 + 1] + 8*2);
		}

		//DebugManager::message("Hi Ma!");
	}
};