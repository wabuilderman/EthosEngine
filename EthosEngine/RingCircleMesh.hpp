#pragma once
#include "Mesh.hpp"

class RingCircleMesh : public Mesh {
public:
	void subdivide(std::vector<glm::vec2>& points) {
		size_t numPoints = points.size();
		points.resize(numPoints * 2);

		for (size_t i = numPoints; i > 0; --i) {
			points[i * 2 - 2] = points[i - 1];
			points[i * 2 - 1] = { 0, 0 };
		}

		for (size_t i = 1; i < numPoints; ++i) {
			glm::vec2& start = points[i * 2 - 2];
			glm::vec2& mid = points[i * 2 - 1];
			glm::vec2& end = points[i * 2 - 0];

			mid = glm::normalize(start + end);
		}

		glm::vec2& start = points[numPoints * 2 - 2];
		glm::vec2& mid = points[numPoints * 2 - 1];
		glm::vec2& end = points[0];
		mid = glm::normalize(start + end);
	}

	void pushTri(unsigned i0, unsigned i1, unsigned i2, bool inverse = false) {
		if (inverse) {
			indices.push_back(i1);
			indices.push_back(i2);
			indices.push_back(i0);
		}
		else {
			indices.push_back(i1);
			indices.push_back(i0);
			indices.push_back(i2);
		}
	}

	void pushRect(unsigned i0, unsigned i1, unsigned i2, unsigned i3, bool inverse = false) {
		pushTri(i0, i1, i2, inverse);
		pushTri(i3, i2, i1, inverse);
	}

	RingCircleMesh(float outerRadius, float innerRadius, size_t numSubdiv, glm::vec3 innerColor, glm::vec3 outerColor, float thickness = 0.5f, glm::vec3 meshUp = { 1,0,0 }, glm::vec3 meshForward = { 0,1,0 }) {

		// Simultaneously normal vectors & positions
		std::vector<glm::vec2> points = {
			{0, 1}, {1, 0}, {0, -1}, {-1, 0}
		};

		for (size_t i = 0; i < numSubdiv; ++i) {
			subdivide(points);
		}


		if (innerRadius != 0.f) {
			// Ring
			for (glm::vec2 point : points) {
				Vertex vTopOut;
				Vertex vBottomOut;
				vTopOut.color = glm::vec4(outerColor, 1.0f);
				vTopOut.normal = meshUp;
				vTopOut.pos =
					point.y * outerRadius * meshForward
					+ point.x * outerRadius * glm::cross(meshUp, meshForward);
				vTopOut.texCoord = { 0.0f, 0.0f };
				vBottomOut = vTopOut;
				vTopOut.pos += meshUp * thickness;

				Vertex vTopInner;
				Vertex vBottomInner;
				vTopInner.color = glm::vec4(innerColor, 1.0f);
				vTopInner.normal = meshUp;
				vTopInner.pos =
					point.y * innerRadius * meshForward
					+ point.x * innerRadius * glm::cross(meshUp, meshForward);
				vTopInner.texCoord = { 0.0f, 0.0f };
				vBottomInner = vTopInner;
				vTopInner.pos += meshUp * thickness;

				vertices.push_back(vTopOut);
				vertices.push_back(vTopInner);
				vertices.push_back(vBottomOut);
				vertices.push_back(vBottomInner);
			}
			for (size_t i = 1; i < points.size(); ++i) {
				pushRect((unsigned)i * 4 - 4, (unsigned)i * 4 - 3, (unsigned)i * 4 + 0, (unsigned)i * 4 + 1, false); // Bottom
				pushRect((unsigned)i * 4 - 2, (unsigned)i * 4 - 1, (unsigned)i * 4 + 2, (unsigned)i * 4 + 3, true); // Top

				pushRect((unsigned)i * 4 - 1, (unsigned)i * 4 - 3, (unsigned)i * 4 + 3, (unsigned)i * 4 + 1, true); // Inner
				pushRect((unsigned)i * 4 - 2, (unsigned)i * 4 - 4, (unsigned)i * 4 + 2, (unsigned)i * 4 + 0, false); // Outer

			}
			pushRect((unsigned)points.size() * 4 - 4, (unsigned)points.size() * 4 - 3, 0u, 1u, false);
			pushRect((unsigned)points.size() * 4 - 2, (unsigned)points.size() * 4 - 1, 2u, 3u, true);
			pushRect((unsigned)points.size() * 4 - 1, (unsigned)points.size() * 4 - 3, 3u, 1u, true);
			pushRect((unsigned)points.size() * 4 - 2, (unsigned)points.size() * 4 - 4, 2u, 0u, false);
		}

		else {
			if (thickness == 0) {
				// Circle
				Vertex center;
				center.color = glm::vec4(innerColor, 1.0f);
				center.normal = glm::vec3({ 1.0f, 0.0f, 0.0f });
				center.pos = { 0.0f, 0.0f, 0.0f };
				center.texCoord = { 0.0f, 0.0f };
				vertices.push_back(center);

				for (glm::vec2 point : points) {
					Vertex v;
					v.color = glm::vec4(outerColor, 1.0f);
					v.normal = glm::vec3({ 1.0f, 0.0f, 0.0f });
					v.pos = point.y * outerRadius * meshForward + point.x * outerRadius * glm::cross(meshUp, meshForward);
					v.texCoord = { 0.0f, 0.0f };
					vertices.push_back(v);
				}

				for (size_t i = 2; i < vertices.size(); ++i) {
					pushTri((unsigned int)(i), (unsigned int)(i - 1), 0u);
				}

				pushTri(1u, (unsigned int)(vertices.size() - 1), 0u);
			}
			else {
				// Disk
				Vertex centerBottom, centerTop;
				centerBottom.color = glm::vec4(innerColor, 1.0f);
				centerBottom.normal = meshUp;
				centerBottom.pos = { 0.0f, 0.0f, 0.0f };
				centerBottom.texCoord = { 0.0f, 0.0f };
				centerTop = centerBottom;
				centerTop.pos += thickness * meshUp;
				vertices.push_back(centerBottom);

				for (glm::vec2 point : points) {
					Vertex vBottom;
					vBottom.color = glm::vec4(outerColor, 1.0f);
					vBottom.normal = -meshUp;
					vBottom.pos = point.y * outerRadius * meshForward + point.x * outerRadius * glm::cross(meshUp, meshForward);
					vBottom.texCoord = { 0.0f, 0.0f };
					vertices.push_back(vBottom);
				}

				vertices.push_back(centerTop);
				for (glm::vec2 point : points) {
					Vertex vTop;
					vTop.color = glm::vec4(outerColor, 1.0f);
					vTop.normal = meshUp;
					vTop.pos = point.y * outerRadius * meshForward + point.x * outerRadius * glm::cross(meshUp, meshForward);
					vTop.texCoord = { 0.0f, 0.0f };
					vTop.pos += meshUp * thickness;
					vertices.push_back(vTop);
				}

				// Make Bottom circle
				for (size_t i = 2; i < vertices.size()/2; i++) {
					pushTri((unsigned int)(i), 0u, (unsigned int)(i - 1));
				}
				pushTri(1u, 0u, (unsigned int)((vertices.size() / 2) - 1));
				
				// Make Top circle
				for (size_t i = 2 + vertices.size()/2; i < vertices.size(); i ++) {
					pushTri((unsigned int)(i), (unsigned int)(i - 1), vertices.size() / 2);
				}
				pushTri((vertices.size() / 2) + 1u, (unsigned int)(vertices.size() - 1), (vertices.size() / 2));

				// Make ribbon
				for (size_t i = 2; i < vertices.size() / 2; i++) {
					pushRect((unsigned int)(i), (unsigned int)(i - 1), (unsigned int)(i + (vertices.size() / 2)), (unsigned int)(i - 1 + (vertices.size() / 2)));
				}
				pushRect(1u, (unsigned int)(vertices.size() / 2) - 1u, (unsigned int)(vertices.size() / 2) + 1u, (unsigned int)(vertices.size()) - 1u);
			}
			
		}
	}
};
