#pragma once
#include "pch.hpp"

#include "Mesh.hpp"
#include <map>

class DefaultMesh : public Mesh {
private:
	static constexpr unsigned numSubdivisions = 2;

	using Index = uint32_t;
	using Lookup = std::map<std::pair<Index, Index>, Index>;

	glm::vec3 blendColors(glm::vec3 a, glm::vec3 b) {
		return {
			(a.x + b.x)/2,
			(a.y + b.y)/2,
			(a.z + b.z)/2
		};
	}

	Index subdivideEdge(Lookup& lookup, Index first, Index second) {
		Lookup::key_type key(first, second);
		if (key.first > key.second)
			std::swap(key.first, key.second);

		auto inserted = lookup.insert({ key, (Index)vertices.size() });
		if (inserted.second) {
			auto& v0 = vertices[first];
			auto& v1 = vertices[second];
			vertices.push_back({
				glm::normalize(v0.pos + v1.pos),
				glm::normalize(v0.normal + v1.normal),
				glm::normalize(v0.tangent + v1.tangent),
				glm::vec4(blendColors(v0.color, v1.color),1.0f),
				glm::normalize(v0.texCoord + v1.texCoord) });
		}

		return inserted.first->second;
	}

	void subdivide() {
		Lookup lookup;
		size_t numTri = indices.size() / 3;

		std::vector<Index> oldIndices;
		oldIndices.swap(indices);
		indices.reserve(numTri * 12);

		for (size_t i = 0; i < numTri; ++i) {
			std::array<Index, 3> mid;
			for (int edge = 0; edge < 3; ++edge) {
				mid[edge] = subdivideEdge(lookup, oldIndices[i * 3 + edge], oldIndices[i * 3 + ((edge + 1) % 3)]);
			}

			indices.push_back(oldIndices[i * 3 + 0]);
			indices.push_back(mid[0]);
			indices.push_back(mid[2]);
			
			

			indices.push_back(oldIndices[i * 3 + 1]);
			indices.push_back(mid[1]);
			indices.push_back(mid[0]);
			
			

			indices.push_back(oldIndices[i * 3 + 2]);
			indices.push_back(mid[2]);
			indices.push_back(mid[1]);
			
			

			indices.push_back(mid[0]);
			indices.push_back(mid[1]);
			indices.push_back(mid[2]);
			
		}
	}

	void initIcosphere() {
		const float X = 0.525731112119133606f;
		const float Z = 0.850650808352039932f;
		const float N = 0.0f;
		const glm::vec2 uv = { 0.0f, 0.0f };
		const glm::vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };

		// TODO: fix tangents

		vertices = {
			{{-X, N, Z},{-X, N, Z},{-X, Z, N}, color, uv},
			{{ X, N, Z},{ X, N, Z},{ X, Z, N}, color, uv},
			{{-X, N,-Z},{ X, N, Z},{ X, Z, N}, color, uv},
			{{ X, N,-Z},{ X, N,-Z},{ X, Z,-N}, color, uv},
			{{ N, Z, X},{ N, Z, X},{ Z, N, X}, color, uv},
			{{ N, Z,-X},{ N, Z,-X},{ Z, N,-X}, color, uv},
			{{ N,-Z, X},{ N,-Z, X},{ Z,-N, X}, color, uv},
			{{ N,-Z,-X},{ N,-Z,-X},{ Z,-N,-X}, color, uv},
			{{ Z, X, N},{ Z, X, N},{ N, X, Z}, color, uv},
			{{-Z, X, N},{-Z, X, N},{-N, X, Z}, color, uv},
			{{ Z,-X, N},{ Z,-X, N},{ N,-X, Z}, color, uv},
			{{-Z,-X, N},{-Z,-X, N},{-N,-X, Z}, color, uv}
		};

		for (unsigned i = 0; i < 12; ++i) {
			vertices[i].color = glm::vec4((glm::vec3(0.5f, 0.5f, 0.5f) + vertices[i].pos) * 0.5f, 1.0f);
		}

		indices = {
			0,1,4,   0,4,9,   9,4,5,   4,8,5,   4,1,8,
			8,1,10,  8,10,3,  5,8,3,   5,3,2,   2,3,7,
			7,3,10,  7,10,6,  7,6,11,  11,6,0,  0,6,1,
			6,10,1,  9,11,0,  9,2,11,  9,5,2,   7,11,2
		};

		for (unsigned i = 0; i < numSubdivisions; ++i) {
			subdivide();
		}
	}

	void addTri(size_t i0, size_t i1, size_t i2) {
		indices.push_back((uint32_t)i0);
		indices.push_back((uint32_t)i1);
		indices.push_back((uint32_t)i2);
	}

	void initUVSphere(unsigned segments, unsigned rings) {

		if (segments < 3 || rings < 3) {
			return;
		}

		float pi2 = 6.28318530718f;
		const glm::vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };

		for (unsigned curSeg = 0; curSeg < segments; ++curSeg) {
			float seg0 = ((float)curSeg) / segments;
			float seg1 = ((float)curSeg + 1) / segments;
			float phi0 = pi2 * seg0;
			float phi1 = pi2 * seg1;

			for (unsigned curRing = 0; curRing < rings; ++curRing) {
				float ring0 = ((float)curRing) / rings;
				float ring1 = ((float)curRing + 1) / rings;
				float theta0 = pi2 * (ring0 - 0.5f) / 2;
				float theta1 = pi2 * (ring1 - 0.5f) / 2;

				glm::vec2 uv00 = { seg0,  ring0 };
				glm::vec2 uv10 = { seg1,  ring0 };
				glm::vec2 uv01 = { seg0,  ring1 };
				glm::vec2 uv11 = { seg1,  ring1 };

				glm::vec3 norm00 = { cosf(theta0) * cosf(phi0), cosf(theta0) * sinf(phi0), sinf(theta0) };
				glm::vec3 norm10 = { cosf(theta1) * cosf(phi0), cosf(theta1) * sinf(phi0), sinf(theta1) };
				glm::vec3 norm01 = { cosf(theta0) * cosf(phi1), cosf(theta0) * sinf(phi1), sinf(theta0) };
				glm::vec3 norm11 = { cosf(theta1) * cosf(phi1), cosf(theta1) * sinf(phi1), sinf(theta1) };

				glm::vec3 tan00 = { -sinf(phi0), cosf(phi0), 0 };
				glm::vec3 tan10 = { -sinf(phi0), cosf(phi0), 0 };
				glm::vec3 tan01 = { -sinf(phi1), cosf(phi1), 0 };
				glm::vec3 tan11 = { -sinf(phi1), cosf(phi1), 0 };

				if (curRing != 0 && curRing != (rings - 1)) {
					vertices.push_back({ norm00 * 0.5f, norm00, tan00, color, uv00 });
					vertices.push_back({ norm10 * 0.5f, norm10, tan10, color, uv10 });
					vertices.push_back({ norm01 * 0.5f, norm01, tan01, color, uv01 });
					vertices.push_back({ norm11 * 0.5f, norm11, tan11, color, uv11 });

					indices.push_back((uint32_t)vertices.size() - 3);
					indices.push_back((uint32_t)vertices.size() - 4);
					indices.push_back((uint32_t)vertices.size() - 2);

					indices.push_back((uint32_t)vertices.size() - 2);
					indices.push_back((uint32_t)vertices.size() - 1);
					indices.push_back((uint32_t)vertices.size() - 3);
				}
				else {
					if (curRing != 0) {
						vertices.push_back({ norm00 * 0.5f, norm00, tan00, color, uv00 });
						vertices.push_back({ norm10 * 0.5f, norm10, tan10, color, uv10 });
						vertices.push_back({ norm01 * 0.5f, norm01, tan01, color, uv01 });

						indices.push_back((uint32_t)vertices.size() - 2);
						indices.push_back((uint32_t)vertices.size() - 3);
						indices.push_back((uint32_t)vertices.size() - 1);
					}
					else {
						vertices.push_back({ norm10 * 0.5f, norm10, tan10, color, uv10 });
						vertices.push_back({ norm01 * 0.5f, norm01, tan01, color, uv01 });
						vertices.push_back({ norm11 * 0.5f, norm11, tan11, color, uv11 });

						indices.push_back((uint32_t)vertices.size() - 2);
						indices.push_back((uint32_t)vertices.size() - 1);
						indices.push_back((uint32_t)vertices.size() - 3);
					}
				}
			}
		}
	}

public:
	static DefaultMesh* get() {
		static DefaultMesh mesh = DefaultMesh();
		return &mesh;
	}

	DefaultMesh() {
		initUVSphere(16*numSubdivisions, 8 * numSubdivisions);
	}
};