#pragma once
#include "Mesh.hpp"
#include "ComplexCubicBezierCurve.hpp"
#include "Transform.hpp"
class GenerativeIntersectionMesh : public Mesh {

public:
	std::vector<glm::vec3> edgePoints;
	std::vector<glm::vec3> edgeVectors;

	size_t resolution;
	float thickness;

	// Flags to toggle each side of the curve mesh
	bool drawBottom = true;
	bool drawTop = true;
	bool drawLeft = true;
	bool drawRight = true;


	void generateCap() {
		std::vector<size_t> cap_indices;
		for (size_t i = 0; i < (edgePoints.size() / 2); ++i) {
			for (size_t j = 0; j <= resolution; ++j) {
				cap_indices.push_back((resolution + 1)*2 + i * 3 * (resolution+1) + j);
			}
		}

		glm::vec3 center = { 0,0,0 };
		float minX = edgePoints.back().x;
		float maxX = minX;
		float minY = edgePoints.back().y;
		float maxY = minY;
		for (glm::vec3 p : edgePoints) {
			center += p * (1.f/edgePoints.size());
			if (p.x < minX) {
				minX = p.x;
			}
			if (p.x > maxX) {
				maxX = p.x;
			}
			if (p.y < minY) {
				minY = p.y;
			}
			if (p.y > maxY) {
				maxY = p.y;
			}
		}
		center += Vector::up * thickness;
		glm::vec2 minCoord = { minX, minY };
		glm::vec2 maxCoord = { maxX, maxY };
		
		Vertex v1 = {
			.pos = center,
			.normal = Vector::up,
			.tangent = Vector::right,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {0.f, 0.0f}
		};
		vertices.push_back(v1);

		for (Vertex& v : vertices) {
			v.normal = Vector::up;
			v.tangent = Vector::right;
			v.texCoord = (glm::vec2({ v.pos.x, v.pos.y }) - minCoord) / (maxCoord-minCoord);
		}

		size_t mid_index = vertices.size() - 1ull;

		for (size_t i = 1; i < cap_indices.size(); ++i) {
			indices.push_back(cap_indices[i-1ull]);
			indices.push_back(cap_indices[i]);
			indices.push_back(mid_index);
		}

		indices.push_back(cap_indices[cap_indices.size() - 1ull]);
		indices.push_back(cap_indices[0]);
		indices.push_back(mid_index);


		return;
	}

	void generatePerimeterCurveSideRibbon(glm::vec3 start, glm::vec3 corner, glm::vec3 end) {
		ComplexCubicBezierCurve curve(start, corner, corner, end);

		glm::vec3 prevPos = start;
		glm::vec3 tan = end - start;
		Vertex v1 = {
			.pos = prevPos,
			.normal = glm::cross(Vector::up, tan),
			.tangent = tan,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {0.f, 0.0f}
		};
		vertices.push_back(v1);
		Vertex v2 = {
			.pos = prevPos + thickness * Vector::up,
			.normal = glm::cross(Vector::up, tan),
			.tangent = tan,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {0.f, 1.0f}
			};
		vertices.push_back(v2);

		for (size_t i = 0; i < resolution; ++i) {
			float t = (i + 1.f) / resolution;
			glm::vec3 curPos = curve.interpolate(t);
			glm::vec3 tan = prevPos - curPos;

			Vertex v1 = {
			.pos = curPos,
			.normal = glm::cross(Vector::up, tan),
			.tangent = tan,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {t, 0.0f}
			};
			vertices.push_back(v1);
			Vertex v2 = {
			.pos = curPos + thickness*Vector::up,
			.normal = glm::cross(Vector::up, tan),
			.tangent = tan,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {t, 1.0f}
			};
			vertices.push_back(v2);

			prevPos = curPos;

			indices.push_back((uint32_t)vertices.size() - 3);
			indices.push_back((uint32_t)vertices.size() - 2);
			indices.push_back((uint32_t)vertices.size() - 1);


			indices.push_back((uint32_t)vertices.size() - 4);
			indices.push_back((uint32_t)vertices.size() - 2);
			indices.push_back((uint32_t)vertices.size() - 3);
		}
		prevPos = start;
		vertices.push_back(v2);
		for (size_t i = 0; i < resolution; ++i) {
			float t = (i + 1.f) / resolution;
			glm::vec3 curPos = curve.interpolate(t);
			glm::vec3 tan = prevPos - curPos;

			Vertex v2 = {
			.pos = curPos + thickness * Vector::up,
			.normal = glm::cross(Vector::up, tan),
			.tangent = tan,
			.color = {1.0f, 1.0f, 1.0f, 1.0f},
			.texCoord = {t, 1.0f}
			};
			vertices.push_back(v2);

			prevPos = curPos;
		}
	}

	void calculateMeshData() {

		//vertices.resize(vertices.size() * (edgePoints.size()/2) + resolution * 2);
		vertices.clear();
		indices.clear();

		if (edgePoints.empty()) {
			// isolated
			RingCircleMesh tempMesh(1.f, 0.f, 5, glm::vec3(1, 1, 1), glm::vec3(1, 1, 1), thickness, Vector::up, Vector::forward);
			for (Vertex v : tempMesh.vertices) {
				vertices.push_back(v);
			}
			for (uint32_t index : tempMesh.indices) {
				indices.push_back(index);
			}
			return;
		}
		else if (edgePoints.size() <= 2) {
			// placed at end of road
			RingCircleMesh tempMesh(1.f, 0.f, 5, glm::vec3(1, 1, 1), glm::vec3(1, 1, 1), thickness, Vector::up, Vector::forward);
			for (Vertex v : tempMesh.vertices) {
				vertices.push_back(v);
			}
			for (uint32_t index : tempMesh.indices) {
				indices.push_back(index);
			}
			return;
		}


		for (size_t i = 1; i < (edgePoints.size()/2); ++i) {
			//glm::vec3 prevTan = glm::normalize(glm::cross(edgePoints[i * 2 - 2] - edgePoints[i * 2 - 2], Vector::up));
			//glm::vec3 nextTan;

			glm::vec3 p1 = edgePoints[i * 2 - 1];
			glm::vec3 p2 = edgePoints[i * 2];

			glm::vec3 v1 = glm::normalize(edgeVectors[i - 1]);
			glm::vec3 v2 = glm::normalize(edgeVectors[i]);

			glm::vec3 p1p2 = p2 - p1;
			float det = (v1.x * v2.y) - (v1.y * v2.x);
			if (det == 0) {
				glm::vec3 tan = -glm::normalize(p1p2);
				glm::vec3 corner = 0.5f * (p1 + p2) + glm::cross(tan, Vector::up) * glm::length(p1p2) * 0.5f;
				generatePerimeterCurveSideRibbon(p1, corner, p2);
			}
			else {
				float t = (p1p2.x * v2.y) - (p1p2.y * v2.x) / det;

				glm::vec3 corner = p1 + v1 * t;

				generatePerimeterCurveSideRibbon(p1, corner, p2);
			}
			
		}

		glm::vec3 p1 = edgePoints.back();
		glm::vec3 p2 = edgePoints.front();

		glm::vec3 v1 = glm::normalize(edgeVectors.back());
		glm::vec3 v2 = glm::normalize(edgeVectors.front());//Vector::forward;//edgeVectors.front();

		glm::vec3 p1p2 = p2 - p1;
		float det = (v1.x * v2.y) - (v1.y * v2.x);
		if (det == 0) {
			glm::vec3 tan = -glm::normalize(p1p2);
			glm::vec3 corner = 0.5f * (p1 + p2) + glm::cross(tan, Vector::up) * glm::length(p1p2) * 0.5f;
			generatePerimeterCurveSideRibbon(p1, corner, p2);
		}
		else {
			float t = (p1p2.x * v2.y) - (p1p2.y * v2.x) / det;
			glm::vec3 corner = p1 + v1 * t;
			generatePerimeterCurveSideRibbon(p1, corner, p2);
		}

		generateCap();

	}

	GenerativeIntersectionMesh(std::vector<glm::vec3> edgePoints, std::vector<glm::vec3> edgeVectors, size_t resolution, float thickness = 0.2f)
		: edgePoints(edgePoints), edgeVectors(edgeVectors), resolution(resolution), thickness(thickness) {
		calculateMeshData();
	}
};