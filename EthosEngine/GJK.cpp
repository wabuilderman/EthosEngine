#include "GJK.hpp"
#include <exception>
#include <vector>

GJK::Simplex GJK::Simplex::Union(const Simplex& A, const glm::vec3& vertex) {
	Simplex result = A;

	if (A.dim == 4) {
		result.vertices[0] = result.vertices[1];
		result.vertices[1] = result.vertices[2];
		result.vertices[2] = result.vertices[3];
		result.dim = 3;
	}

	result.vertices[result.dim] = vertex;

	++result.dim;
	return result;
}

GJK::Simplex GJK::Simplex::Conv(const glm::vec3& v, const Simplex& S) {
	Simplex result;
	result.dim = S.dim;
	for (unsigned i = 0; i < S.dim; ++i) {
		result.vertices[i] = (glm::vec3)(v - S.vertices[i]);
	}
	return result;
}

float GJK::Simplex::maxDist(glm::vec3 v) {
	float result = 0.0f;
	for (unsigned i = 0; i < dim; ++i) {
		result = glm::max(glm::length2(v - vertices[i]), result);
	}
	return result;
}

bool GJK::Simplex::equals(const Simplex& S) {
	if (dim != S.dim) { return false; }
	for (unsigned i = 0; i < dim; ++i) {
		if (glm::length2(vertices[i] - S.vertices[i]) < (FLT_EPSILON) * (FLT_EPSILON)) {
			return true;
		}
	}
	return false;
}

bool GJK::Simplex::contains(const glm::vec3& v) {
	for (unsigned i = 0; i < dim; ++i) {
		if (glm::length2(vertices[i] - v) < (FLT_EPSILON) * (FLT_EPSILON)) {
			return true;
		}
	}
	return false;
}

inline void GJK::Simplex::assign(unsigned dst, unsigned src) {
	vertices[dst] = vertices[src];
}

inline void GJK::Simplex::swap(unsigned a, unsigned b) {
	glm::vec3 tmpVertex = vertices[a];
	vertices[a] = vertices[b];
	vertices[b] = tmpVertex;
}

/*
 * Set closestPoint to the point closest to the origin within Q
 * Set Q to be the simplest simplex containing closestPoint
 */
void GJK::doLine(Simplex& Q, glm::vec3& closestPoint) {
	// a = Q.vertices[1];
	// b = Q.vertices[0];
	const glm::vec3& a = Q.vertices[1];
	const glm::vec3& b = Q.vertices[0];

	glm::vec3 ab = b - a;
	float origProj = -glm::dot(ab, a);
	if (origProj > 0) {
		float lengthSqr = glm::dot(ab, ab);
		if (origProj < lengthSqr) {
			closestPoint = (origProj / lengthSqr) * ab + a;
			Q.dim = 2;
		}
		else {
			closestPoint = b;
			Q.dim = 1;
		}

	} else {
		closestPoint = a;
		Q.assign(0, 1);
		Q.dim = 1;
	}
}

/*
 * Set closestPoint to the point closest to the origin within Q
 * Set Q to be the simplest simplex containing closestPoint
 */
void GJK::doTriangle(Simplex& Q, glm::vec3& closestPoint) {
	const glm::vec3& a = Q.vertices[2];
	const glm::vec3& b = Q.vertices[1];
	const glm::vec3& c = Q.vertices[0];

	glm::vec3 ba = b - a;
	glm::vec3 cb = c - b;
	glm::vec3 ca = c - a;

	// compute barycentric coordinates
	float u_ab = glm::dot(b, ba);
	float v_ab = glm::dot(a, -ba);

	float u_bc = glm::dot(c, cb);
	float v_bc = glm::dot(b, -cb);

	float u_ca = glm::dot(a, -ca);
	float v_ca = glm::dot(c, ca);

	if (v_ab <= 0.0f && u_ca <= 0.0f) {
		// region A
		closestPoint = a;
		Q.assign(0, 2);
		Q.dim = 1;
		
	}
	else if (u_ab <= 0.0f && v_bc <= 0.0f) {
		// region B
		closestPoint = b;
		Q.assign(0, 1);
		Q.dim = 1;
		
	}
	else if (u_bc <= 0.0f && v_ca <= 0.0f) {
		// region C
		closestPoint = c;
		Q.dim = 1;
	}
	else {
		// calculate fractional area
		glm::vec3 n = glm::cross(ba, ca);
		glm::vec3 n1 = glm::cross(b, c);
		glm::vec3 n2 = glm::cross(c, a);
		glm::vec3 n3 = glm::cross(a, b);

		float u_abc = glm::dot(n1, n);
		float v_abc = glm::dot(n2, n);
		float w_abc = glm::dot(n3, n);

		if (u_ab > 0.0f && v_ab > 0.0f && w_abc <= 0.0f) {
			// region AB
			closestPoint = a - (glm::dot(ba, a) / glm::dot(ba, ba)) * ba;
			Q.assign(0, 2);
			Q.dim = 2;
		}
		else if (u_bc > 0.0f && v_bc > 0.0f && u_abc <= 0.0f) {
			// region BC
			closestPoint = b - (glm::dot(cb, b) / glm::dot(cb, cb)) * cb;
			Q.dim = 2;
		}
		else if (u_ca > 0.0f && v_ca > 0.0f && v_abc <= 0.0f) {
			// region CA
			closestPoint = a - (glm::dot(ca, a) / glm::dot(ca, ca)) * ca;
			Q.assign(1, 2);
			Q.dim = 2;
		}
		else {
			// region ABC
			assert(u_abc > 0.0f && v_abc > 0.0f && w_abc > 0.0f);
			n = glm::normalize(n);
			closestPoint = glm::dot(c, n) * n;
		}
	}
}

/*
 * Set closestPoint to the point closest to the origin within Q
 * Set Q to be the simplest simplex containing closestPoint
 */
void GJK::doTetrahedron(Simplex& Q, glm::vec3& closestPoint) {
	// https://www.geometrictools.com/Documentation/DistanceLineTetrahedron.pdf
	
	const glm::vec3& a = Q.vertices[3];
	const glm::vec3& b = Q.vertices[2];
	const glm::vec3& c = Q.vertices[1];
	const glm::vec3& d = Q.vertices[0];

	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ad = d - a;

	glm::mat3x3 E = { ab.x, ab.y, ab.z, ac.x, ac.y, ac.z, ad.x, ad.y, ad.z };
	//glm::mat3x3 E = { ab.x, ac.x, ad.x, ab.y, ac.y, ad.y, ab.z, ac.z, ad.z };
	// For any vector s, such that 0 <= s.x <= 1, 0 <= s.y <= 1, 0 <= s.z <= 1, & 0 <= s.x + s.y + s.z <= 1
	// Q.vertices[3] + s * E is a point within the tetrahedron

	// To test whether the origin is within the tetrahedron:
	// (0, 0, 0) = Q.vertices[3] + s * E
	// s = -Q.vertices[3] * E^-1
	// Test s for preivously mentioned inequalities

	float det = glm::determinant(E);
	if (fabsf(det) < FLT_EPSILON) {
		/*glm::vec3 normal = glm::cross(ab, ac);
		if (glm::dot(normal, normal) < FLT_EPSILON * FLT_EPSILON) {
			closestPoint = glm::vec3(0, 0, 0);
		}
		else {
			closestPoint = glm::normalize(normal) * FLT_EPSILON * FLT_EPSILON * 0.5f;
		}*/
		Q.dim = 3;
		return GJK::doTriangle(Q, closestPoint);
	}

	glm::vec3 s = -(glm::inverse(E) * Q.vertices[3]);

	if (s.x + s.y + s.z >= 1) {
		if (s.z > 0) {
			if (s.y > 0) {
				if (s.x > 0) {
					// Triangle BCD closest
					Q.dim = 3;
					GJK::doTriangle(Q, closestPoint);
				}
				else {
					// Line CD closest
					Q.dim = 2;
					GJK::doLine(Q, closestPoint);
				}
			}
			else {
				if (s.x > 0) {
					// B is closest
					closestPoint = b;
					Q.assign(0, 2);
					Q.dim = 1;
					
				}
				else {
					// D is closest
					closestPoint = d;
					Q.dim = 1;
				}
			}
		}
		else {
			if (s.y > 0) {
				if (s.x > 0) {
					// Line BC closest
					Q.assign(0, 1);
					Q.assign(1, 2);
					Q.dim = 2;
					GJK::doLine(Q, closestPoint);
				}
				else {
					// C is closest
					Q.assign(0, 1);
					Q.dim = 1;
					closestPoint = c;
				}
			}
			else {
				// Line BD closest
				Q.assign(1, 2);
				Q.dim = 2;
				GJK::doLine(Q, closestPoint);
			}
		}
	}
	else {
		if (s.z > 0) {
			if (s.y > 0) {
				if (s.x > 0) {
					// Inside tetrahedron
					Q.dim = 4;
					closestPoint = { 0, 0, 0 };
				}
				else {
					// Triangle ACD closest
					Q.assign(2, 3);
					Q.dim = 3;
					GJK::doTriangle(Q, closestPoint);
				}
			}
			else {
				if (s.x > 0) {
					// Triangle ABD closest
					Q.assign(1, 2);
					Q.assign(2, 3);
					Q.dim = 3;
					GJK::doTriangle(Q, closestPoint);
				}
				else {
					// Line AD closest
					Simplex testQ1 = Q;
					glm::vec3 lineP;
					testQ1.assign(1, 3);
					testQ1.dim = 2;
					GJK::doLine(testQ1, lineP);

					// Triangle ACD closest
					Simplex testQ2 = Q;
					glm::vec3 acdP;
					testQ2.assign(2, 3);
					testQ2.dim = 3;
					GJK::doTriangle(testQ2, acdP);

					if (glm::length2(acdP) < glm::length2(lineP)) {
						Q = testQ2;
						closestPoint = acdP;
					}
					else {
						Q = testQ1;
						closestPoint = lineP;
					}
				}
			}
		}
		else {
			if (s.y > 0) {
				if (s.x > 0) {
					// Triangle ABC closest
					Q.assign(0, 1);
					Q.assign(1, 2);
					Q.assign(2, 3);
					Q.dim = 3;
					GJK::doTriangle(Q, closestPoint);
				}
				else {
					// Line AC closest
					Q.assign(0, 1);
					Q.assign(1, 3);
					Q.dim = 2;
					GJK::doLine(Q, closestPoint);
				}
			}
			else {
				if (s.x > 0) {
					// Line AB closest
					Q.assign(0, 2);
					Q.assign(1, 3);
					Q.dim = 2;
					GJK::doLine(Q, closestPoint);
				}
				else {
					// A is closest
					closestPoint = a;
					Q.assign(0, 3);
					Q.dim = 1;
				}
			}
		}
	}
}

/*
 * Set closestPoint to the point closest to the origin within Q
 * Set Q to be the simplest subset simplex containing closestPoint
 * Return whether or not the simplex contains the origin
 */
bool GJK::doSimplex(Simplex& Q, glm::vec3& closestPoint) {
	switch (Q.dim) {
		case 1: closestPoint = Q.vertices[0]; break; // Point Case - trivial
		case 2: GJK::doLine(Q, closestPoint); break; // Line Case
		case 3: GJK::doTriangle(Q, closestPoint); break; // Triangle Case
		case 4: GJK::doTetrahedron(Q, closestPoint); break; // Tetrahedron Case
		default: throw std::exception("Incorrect number of dimensions");
	}

	return (glm::length2(closestPoint) < FLT_EPSILON * FLT_EPSILON * 2);
}

glm::vec3 swapAxis(glm::vec3 v, unsigned k) {
	switch (k) {
	case 0: // xyz
		return { v.x, v.y, v.z };
	case 1: // xzy
		return { v.x, v.z, v.y };
	case 2: // yxz
		return { v.y, v.x, v.z };
	case 3: // yzx
		return { v.y, v.z, v.x };
	case 4: // zyx
		return { v.z, v.y, v.x };
	case 5: // zxy
		return { v.z, v.x, v.y };
	}
}

GJK::Simplex swapAxis(const GJK::Simplex& Q, unsigned k) {
	return { { swapAxis(Q.vertices[0], k), swapAxis(Q.vertices[1], k), swapAxis(Q.vertices[2], k), swapAxis(Q.vertices[3], k) }, Q.dim };
}

void test_doSimplex(GJK::Simplex Q, glm::vec3 expectedVec, bool expectedBool, const GJK::Simplex& expectedQ) {
	glm::vec3 v = { 0, 0, 0 };
	bool result = GJK::doSimplex(Q, v);
	assert(glm::length2(v - expectedVec) < FLT_EPSILON*FLT_EPSILON*4);
	assert(result == expectedBool);
	assert(Q.dim == expectedQ.dim);
	for (unsigned i = 0; i < Q.dim; ++i) {
		// assert that the point is *somewhere* in the expected, order is not important. Repeats are unimportant, as they should always be eliminated
		bool contained = false;
		for (unsigned j = 0; j < Q.dim; ++j) {
			contained |= Q.vertices[i] == expectedQ.vertices[j];
		}
		assert(contained);
	}
}

// Runs a test with the vertices of Q interchanged
void test_doSimplex_symOrder(const GJK::Simplex& Q, glm::vec3 expectedVec, bool expectedBool, const GJK::Simplex& expectedQ) {
	if (Q.dim > 1) {
		test_doSimplex({ {Q.vertices[0], Q.vertices[1], Q.vertices[2], Q.vertices[3]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[1], Q.vertices[0], Q.vertices[2], Q.vertices[3]}, Q.dim }, expectedVec, expectedBool, expectedQ);
	}

	if (Q.dim > 2) {
		test_doSimplex({ {Q.vertices[0], Q.vertices[2], Q.vertices[1], Q.vertices[3]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[1], Q.vertices[2], Q.vertices[0], Q.vertices[3]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[2], Q.vertices[1], Q.vertices[0], Q.vertices[3]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[2], Q.vertices[0], Q.vertices[1], Q.vertices[3]}, Q.dim }, expectedVec, expectedBool, expectedQ);
	}

	if (Q.dim > 3) {
		test_doSimplex({ {Q.vertices[0], Q.vertices[1], Q.vertices[3], Q.vertices[2]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[0], Q.vertices[2], Q.vertices[3], Q.vertices[1]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[0], Q.vertices[3], Q.vertices[2], Q.vertices[1]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[0], Q.vertices[3], Q.vertices[1], Q.vertices[2]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[1], Q.vertices[0], Q.vertices[3], Q.vertices[2]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[1], Q.vertices[2], Q.vertices[3], Q.vertices[0]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[1], Q.vertices[3], Q.vertices[2], Q.vertices[0]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[1], Q.vertices[3], Q.vertices[0], Q.vertices[2]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[2], Q.vertices[1], Q.vertices[3], Q.vertices[0]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[2], Q.vertices[0], Q.vertices[3], Q.vertices[1]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[2], Q.vertices[3], Q.vertices[0], Q.vertices[1]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[2], Q.vertices[3], Q.vertices[1], Q.vertices[0]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[3], Q.vertices[1], Q.vertices[2], Q.vertices[0]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[3], Q.vertices[1], Q.vertices[0], Q.vertices[2]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[3], Q.vertices[2], Q.vertices[0], Q.vertices[1]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[3], Q.vertices[2], Q.vertices[1], Q.vertices[0]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[3], Q.vertices[0], Q.vertices[2], Q.vertices[1]}, Q.dim }, expectedVec, expectedBool, expectedQ);
		test_doSimplex({ {Q.vertices[3], Q.vertices[0], Q.vertices[1], Q.vertices[2]}, Q.dim }, expectedVec, expectedBool, expectedQ);
	}
}

// Runs a test with the vertices of Q interchanged, as well as with the x, y, and z axis interchanged
void test_doSimplex_symAxisOrder(const GJK::Simplex& Q, glm::vec3 expectedVec, bool expectedBool, const GJK::Simplex& expectedQ) {
	for (unsigned i = 0; i < 6; ++i) {
		test_doSimplex_symOrder(swapAxis(Q, i), swapAxis(expectedVec, i), expectedBool, swapAxis(expectedQ, i));
	}
}

void test_doSimplex_wrapper(std::vector<glm::vec3> points, glm::vec3 expectedPoint, unsigned expectedDim) {
	GJK::Simplex Q;
	Q.dim = points.size();
	for (size_t i = 0; i < points.size() && i < 4; ++i) {
		Q.vertices[i] = points.at(i);
	}
	assert(expectedDim <= Q.dim);

	test_doSimplex_symAxisOrder(Q, expectedPoint, (expectedPoint == glm::vec3(0,0,0)), { {Q.vertices[0],Q.vertices[1],Q.vertices[2],Q.vertices[3]}, expectedDim });
}

void GJK::runUnitTest() {

	// Line ------------------------------
	test_doSimplex_wrapper({ { 1, 0, 0 }, { 0, 1, 0 } }, { 0.5f, 0.5f, 0 }, 2); // 2-axis slant midpoint
	test_doSimplex_wrapper({ { 1, 0, 0 }, { 0, 0.5, 0 } }, { 0.2f, 0.4f, 0 }, 2); // 2-axis slant off-midpoint
	test_doSimplex_wrapper({ { 0.5, 0.5, 0 }, { 0, 1, 0 } }, { 0.5f, 0.5f, 0 }, 1); // 2-axis slant endpoint
	test_doSimplex_wrapper({ { 0.4, 0.6, 0 }, { 0, 1, 0 } }, { 0.4f, 0.6f, 0 }, 1); // 2-axis slant past endpoint
	test_doSimplex_wrapper({ { 1, 1, 0 }, { 2, 2, 0 } }, { 1, 1, 0 }, 1); // 2-axis aligned

	// Triangle ------------------------------
	// axis-plane
	test_doSimplex_wrapper({ { 0,0,0 }, { 1,0,0 }, { 0,1,0 } }, { 0, 0, 0 }, 1); // origin contained in A
	test_doSimplex_wrapper({ { 1,0,0 }, { -1,0,0 }, { 0,1,0 } }, { 0, 0, 0 }, 2); // origin contained in AB
	test_doSimplex_wrapper({ { 1,0,0 }, { 0,1,0 }, { -1,-1,0 } }, { 0, 0, 0 }, 3); // origin contained in ABC

	// offset above origin
	test_doSimplex_wrapper({ { 0,0,1 }, { 1,0,1 }, { 0,1,1 } }, { 0, 0, 1 }, 1); // origin contained in A
	test_doSimplex_wrapper({ { 1,0,1 }, { -1,0,1 }, { 0,1,1 } }, { 0, 0, 1 }, 2); // origin contained in AB
	test_doSimplex_wrapper({ { 1,0,1 }, { 0,1,1 }, { -1,-1,1 } }, { 0, 0, 1 }, 3); // origin contained in ABC
}