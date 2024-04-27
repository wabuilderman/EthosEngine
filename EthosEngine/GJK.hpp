#pragma once
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace GJK {
	struct Simplex {
		glm::vec3 vertices[4] = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };
		unsigned dim = 0;

		static Simplex Union(const Simplex& A, const glm::vec3& vertex);
		static Simplex Conv(const glm::vec3& v, const Simplex& S);

		inline void assign(unsigned dst, unsigned src);
		inline void swap(unsigned a, unsigned b);

		float maxDist(glm::vec3 v);
		bool equals(const Simplex& S);
		bool contains(const glm::vec3& v);
	};

	void doLine(Simplex& Q, glm::vec3& closestPoint);
	void doTriangle(Simplex& Q, glm::vec3& closestPoint);
	void doTetrahedron(Simplex& Q, glm::vec3& closestPoint);
	bool doSimplex(Simplex& Q, glm::vec3& closestPoint);

	void runUnitTest();
}