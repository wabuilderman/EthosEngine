#pragma once
#include "pch.hpp"
#include "Mesh.hpp"
#include "ComplexCubicBezierCurve.hpp"
#include "Transform.hpp"
#include "RenderComponentManager.hpp"

class GenerativeCubicBezierMesh : public Mesh {

public:
	ComplexCubicBezierCurve curve;

	size_t resolution;
	float width;
	float thickness;

	float zOffset;
	float xOffset;

	// Flags to toggle each side of the curve mesh
	bool drawBottom = true;
	bool drawTop = true;
	bool drawLeft = true;
	bool drawRight = true;
	bool drawStartCap = true;
	bool drawEndCap = true;

	void calculateMeshData() {
		// Divide the curve into resolution+1 equal segments (equal in terms of the t-parameter, not distance along curve);
		float prevT = 0;
		glm::vec3 prevMidPos = curve.interpolate(prevT);
		glm::vec3 prevBiNorm = glm::normalize(glm::cross(curve.derivative(prevT), Vector::up));

		size_t vertsPerInc = getVerticesPerIncrementCount();
		size_t topIndex = 0;
		size_t bottomIndex = topIndex + (drawTop ? 2 : 0);
		size_t leftIndex = bottomIndex + (drawBottom ? 2 : 0);
		size_t rightIndex = leftIndex + (drawLeft ? 2 : 0);

		if (drawTop) {
			vertices[topIndex] = {
				.pos = prevMidPos + prevBiNorm * (width / 2.f) + Vector::up * thickness + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {0.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {0.0f, 0.0f}
			};

			vertices[topIndex + 1] = {
				.pos = prevMidPos - prevBiNorm * (width / 2.f) + Vector::up * thickness + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {0.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {0.0f, 1.0f}
			};
		}

		if (drawBottom) {
			vertices[bottomIndex] = {
				.pos = prevMidPos + prevBiNorm * (width / 2.f) + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {0.0f, 0.0f}
			};

			vertices[bottomIndex + 1] = {
				.pos = prevMidPos - prevBiNorm * (width / 2.f) + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {0.0f, 1.0f}
			};
		}

		if (drawLeft) {
			vertices[leftIndex] = {
				.pos = prevMidPos - prevBiNorm * (width / 2.f) + Vector::up * thickness + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {0.0f, 0.f}
			};

			vertices[leftIndex + 1] = {
				.pos = prevMidPos - prevBiNorm * (width / 2.f) + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {0.0f, -thickness}
			};
		}

		if (drawRight) {
			vertices[rightIndex] = {
				.pos = prevMidPos + prevBiNorm * (width / 2.f) + Vector::up * thickness + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {0.0f, 0.0f}
			};

			vertices[rightIndex + 1] = {
				.pos = prevMidPos + prevBiNorm * (width / 2.f) + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {0.0f, thickness}
			};
		}

		glm::vec3 curMidPos;
		float prevTexCoordLength = 0;

		for (size_t i = 1; i <= resolution; ++i) {
			float curT = (i + 1) / (1.0f + resolution);
			// get midpoint of bezier curve at both points
			curMidPos = curve.interpolate(curT);
			float segLength = glm::distance(curMidPos, prevMidPos);

			// get binormal of bezier curve at both points
			glm::vec3 curBiNorm = glm::normalize(glm::cross(curve.derivative(curT), Vector::up));

			size_t offset = i * vertsPerInc;

			if (drawTop) {
				vertices[offset + topIndex] = {
					.pos = curMidPos + curBiNorm * (width / 2.f) + Vector::up * thickness + Vector::up * zOffset,
					.normal = Vector::up,
					.tangent = curBiNorm,
					.color = {curT, 1.0f, 1.0f, 1.0f},
					.texCoord = {prevTexCoordLength + segLength, 0.0f}
				};
				vertices[offset + topIndex + 1] = {
					.pos = curMidPos - curBiNorm * (width / 2.f) + Vector::up * thickness + Vector::up * zOffset,
					.normal = Vector::up,
					.tangent = curBiNorm,
					.color = {curT, 1.0f, 1.0f, 1.0f},
					.texCoord = {prevTexCoordLength + segLength, 1.0f}
				};
			}

			if (drawBottom) {
				vertices[offset + bottomIndex] = {
				.pos = curMidPos + curBiNorm * (width / 2.f) + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = curBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {prevTexCoordLength + segLength, 0.0f}
				};

				vertices[offset + bottomIndex + 1] = {
					.pos = curMidPos - curBiNorm * (width / 2.f) + Vector::up * zOffset,
					.normal = Vector::up,
					.tangent = curBiNorm,
					.color = {1.0f, 1.0f, 1.0f, 1.0f},
					.texCoord = {prevTexCoordLength + segLength, 1.0f}
				};
			}

			if (drawLeft) {
				vertices[offset + leftIndex] = {
					.pos = curMidPos - curBiNorm * (width / 2.f) + Vector::up * thickness + Vector::up * zOffset,
					.normal = Vector::up,
					.tangent = curBiNorm,
					.color = {1.0f, 1.0f, 1.0f, 1.0f},
					.texCoord = {prevTexCoordLength + segLength, 0}
				};
				vertices[offset + leftIndex + 1] = {
					.pos = curMidPos - curBiNorm * (width / 2.f) + Vector::up * zOffset,
					.normal = Vector::up,
					.tangent = curBiNorm,
					.color = {1.0f, 1.0f, 1.0f, 1.0f},
					.texCoord = {prevTexCoordLength + segLength, -thickness}
				};

			}

			if (drawRight) {
				vertices[offset + rightIndex] = {
					.pos = curMidPos + curBiNorm * (width / 2.f) + Vector::up * thickness + Vector::up * zOffset,
					.normal = Vector::up,
					.tangent = curBiNorm,
					.color = {1.0f, 1.0f, 1.0f, 1.0f},
					.texCoord = {prevTexCoordLength + segLength, 0.0f}
				};
				vertices[offset + rightIndex + 1] = {
					.pos = curMidPos + curBiNorm * (width / 2.f) + Vector::up * zOffset,
					.normal = Vector::up,
					.tangent = curBiNorm,
					.color = {1.0f, 1.0f, 1.0f, 1.0f},
					.texCoord = {prevTexCoordLength + segLength, thickness}
				};
			}


			size_t baseOffset = i * getIndicesPerIncrementCount();

			// top face
			if (drawTop) {
				indices[baseOffset++] = (uint32_t)((i - 1) * vertsPerInc + topIndex + 1);
				indices[baseOffset++] = (uint32_t)((i - 1) * vertsPerInc + topIndex + 0);
				indices[baseOffset++] = (uint32_t)((i - 0) * vertsPerInc + topIndex + 0);

				indices[baseOffset++] = (uint32_t)((i - 1) * vertsPerInc + topIndex + 1);
				indices[baseOffset++] = (uint32_t)((i - 0) * vertsPerInc + topIndex + 0);
				indices[baseOffset++] = (uint32_t)((i - 0) * vertsPerInc + topIndex + 1);
			}

			// left face
			if (drawLeft) {
				indices[baseOffset++] = (uint32_t)((i - 1) * vertsPerInc + leftIndex + 1);
				indices[baseOffset++] = (uint32_t)((i - 1) * vertsPerInc + leftIndex + 0);
				indices[baseOffset++] = (uint32_t)((i - 0) * vertsPerInc + leftIndex + 1);

				indices[baseOffset++] = (uint32_t)((i - 1) * vertsPerInc + leftIndex + 0);
				indices[baseOffset++] = (uint32_t)((i - 0) * vertsPerInc + leftIndex + 0);
				indices[baseOffset++] = (uint32_t)((i - 0) * vertsPerInc + leftIndex + 1);
			}


			// right face
			if (drawRight) {
				indices[baseOffset++] = (uint32_t)((i - 1) * vertsPerInc + rightIndex + 0);
				indices[baseOffset++] = (uint32_t)((i - 1) * vertsPerInc + rightIndex + 1);
				indices[baseOffset++] = (uint32_t)((i - 0) * vertsPerInc + rightIndex + 0);

				indices[baseOffset++] = (uint32_t)((i - 1) * vertsPerInc + rightIndex + 1);
				indices[baseOffset++] = (uint32_t)((i - 0) * vertsPerInc + rightIndex + 1);
				indices[baseOffset++] = (uint32_t)((i - 0) * vertsPerInc + rightIndex + 0);
			}

			// bottom face
			if (drawBottom) {
				indices[baseOffset++] = (uint32_t)((i - 1) * vertsPerInc + bottomIndex + 0);
				indices[baseOffset++] = (uint32_t)((i - 1) * vertsPerInc + bottomIndex + 1);
				indices[baseOffset++] = (uint32_t)((i - 0) * vertsPerInc + bottomIndex + 1);

				indices[baseOffset++] = (uint32_t)((i - 1) * vertsPerInc + bottomIndex + 0);
				indices[baseOffset++] = (uint32_t)((i - 0) * vertsPerInc + bottomIndex + 1);
				indices[baseOffset++] = (uint32_t)((i - 0) * vertsPerInc + bottomIndex + 0);
			}

			prevMidPos = curMidPos;
			prevBiNorm = curBiNorm;
			prevTexCoordLength += segLength;
		}


		size_t offset = (resolution + 1) * vertsPerInc;
		size_t indexOffset = (resolution + 1) * getIndicesPerIncrementCount();

		if (drawEndCap) {
			vertices[offset++] = {
				.pos = prevMidPos + prevBiNorm * (width / 2.f) + Vector::up * thickness + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {0.f, 0.0f}
			};
			vertices[offset++] = {
				.pos = prevMidPos - prevBiNorm * (width / 2.f) + Vector::up * thickness + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {0.f, 1.0f}
			};
			vertices[offset++] = {
				.pos = curMidPos + prevBiNorm * (width / 2.f) + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {thickness, 0.0f}
			};
			vertices[offset++] = {
				.pos = curMidPos - prevBiNorm * (width / 2.f) + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {thickness, 1.0f}
			};

			indices[indexOffset++] = (uint32_t)(offset - 3);
			indices[indexOffset++] = (uint32_t)(offset - 4);
			indices[indexOffset++] = (uint32_t)(offset - 2);
			indices[indexOffset++] = (uint32_t)(offset - 3);
			indices[indexOffset++] = (uint32_t)(offset - 2);
			indices[indexOffset++] = (uint32_t)(offset - 1);
		}

		if (drawStartCap) {
			prevMidPos = curve.interpolate(0);
			prevBiNorm = glm::normalize(glm::cross(curve.derivative(0), Vector::up));

			vertices[offset++] = {
					.pos = prevMidPos + prevBiNorm * (width / 2.f) + Vector::up * thickness + Vector::up * zOffset,
					.normal = Vector::up,
					.tangent = prevBiNorm,
					.color = {1.0f, 1.0f, 1.0f, 1.0f},
					.texCoord = {0.0f, 0.0f}
			};
			vertices[offset++] = {
				.pos = prevMidPos - prevBiNorm * (width / 2.f) + Vector::up * thickness + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {0.0f, 1.0f}
			};
			vertices[offset++] = {
				.pos = prevMidPos + prevBiNorm * (width / 2.f) + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {thickness, 0.0f}
			};
			vertices[offset++] = {
				.pos = prevMidPos - prevBiNorm * (width / 2.f) + Vector::up * zOffset,
				.normal = Vector::up,
				.tangent = prevBiNorm,
				.color = {1.0f, 1.0f, 1.0f, 1.0f},
				.texCoord = {thickness, 1.0f}
			};

			indices[indexOffset++] = (uint32_t)(offset - 4);
			indices[indexOffset++] = (uint32_t)(offset - 3);
			indices[indexOffset++] = (uint32_t)(offset - 2);

			indices[indexOffset++] = (uint32_t)(offset - 2);
			indices[indexOffset++] = (uint32_t)(offset - 3);
			indices[indexOffset++] = (uint32_t)(offset - 1);
		}
	}

	inline size_t getIndicesPerIncrementCount() {
		return (drawBottom ? 6ull : 0ull) + (drawTop ? 6ull : 0ull) + (drawLeft ? 6ull : 0ull) + (drawRight ? 6ull : 0ull);
	}
	inline size_t getVerticesPerIncrementCount() {
		return (drawBottom ? 2ull : 0ull) + (drawTop ? 2ull : 0ull) + (drawLeft ? 2ull : 0ull) + (drawRight ? 2ull : 0ull);
	}

	// TODO: implement z and x offsets
	GenerativeCubicBezierMesh(ComplexCubicBezierCurve curve, size_t resolution, float width, float thickness = 0.2f, float zOffset = 0.0f, float xOffset = 0.0f,
		bool drawTop = true, bool drawBottom = true, bool drawLeft = true, bool drawRight = true, bool drawStartCap = true, bool drawEndCap = true
	) : curve(curve), resolution(resolution), width(width), thickness(thickness), zOffset(zOffset), xOffset(xOffset), drawTop(drawTop), drawBottom(drawBottom), drawLeft(drawLeft), drawRight(drawRight), drawStartCap(drawStartCap), drawEndCap(drawEndCap) {
		vertices.resize(getIndicesPerIncrementCount() * (resolution + 1) + (drawStartCap ? 4 : 0) + (drawEndCap ? 4 : 0));
		indices.resize(getIndicesPerIncrementCount() * (resolution + 1) + (drawStartCap ? 6 : 0) + (drawEndCap ? 6 : 0));
		calculateMeshData();
	}
};