#include "Collision.hpp"
#include "PhysicsBody.hpp"

// Returns the support vector for a given direction
glm::vec3 Collision::A_Support_Transformed(const glm::vec3& dir) {
	return bodyA->targetTransform.getPosition() +
		bodyA->targetTransform.getRotation() *
		shapeA->support(dir * bodyA->targetTransform.getRotation());
}

glm::vec3 Collision::B_Support_Transformed(const glm::vec3& dir) {
	return bodyB->targetTransform.getPosition() +
		bodyB->targetTransform.getRotation() *
		shapeB->support(dir * bodyB->targetTransform.getRotation());
}

glm::vec3 Collision::CSO_Support_Transformed(const glm::vec3& dir) {
	glm::vec3 result = A_Support_Transformed(dir) - B_Support_Transformed(-dir);
	return { result.x, result.y, result.z };
}

constexpr unsigned maxPolytopeDim = 96;
constexpr unsigned maxPolytopeVert = 32;

struct Polytope {
	struct Face {
		unsigned a = 0, b = 0, c = 0;
		glm::dvec3 normal = { 0, 0, 0 };
		float origDist = 0.0f;
		float affineness = 0.f;

		bool contains(unsigned i) {
			return i == a || i == b || i == c;
		}
	};

	struct Edge {
		unsigned a, b;
		bool mirrorEquals(const Edge& other) const {
			return (a == other.a && b == other.b) || (a == other.b && b == other.a);
		}
	};

	glm::vec3 vertices[maxPolytopeVert] = {};
	glm::vec3 supports[maxPolytopeVert] = {};
	Face faces[maxPolytopeDim] = {};
	unsigned vertexCount = 0;
	unsigned faceCount = 0;

	void addFace(unsigned v0, unsigned v1, unsigned v2) {
		if (faceCount >= maxPolytopeDim) {
			throw "Max Polytope Dimension Reached";
		}
		Face& f = faces[faceCount];
		f.a = v0;
		f.b = v1;
		f.c = v2;

		glm::dvec3 ab = (glm::dvec3)vertices[f.b] - (glm::dvec3)vertices[f.a];
		glm::dvec3 ac = (glm::dvec3)vertices[f.c] - (glm::dvec3)vertices[f.a];
		glm::dvec3 bc = (glm::dvec3)vertices[f.c] - (glm::dvec3)vertices[f.b];

		glm::dvec3 n1 = glm::cross(ab, ac);
		glm::dvec3 n2 = glm::cross(ab, bc);
		glm::dvec3 n3 = glm::cross(ac, bc);

		glm::dvec3 n1n2Diff = n1 - n2;
		glm::dvec3 n2n3Diff = n2 - n3;
		glm::dvec3 n3n1Diff = n3 - n1;

		double n1n2DiffMagSqr = glm::length2(n1n2Diff);
		double n2n3DiffMagSqr = glm::length2(n2n3Diff);
		double n3n1DiffMagSqr = glm::length2(n3n1Diff);
		f.affineness = (float)glm::length2(glm::cross(ab, ac))*glm::length2(bc);

		f.normal = glm::normalize(glm::cross(ab, ac) + glm::cross(ab, bc) + glm::cross(ac, bc));
		f.origDist = (float)glm::dot((glm::dvec3) vertices[f.b], f.normal);
		if (f.origDist != f.origDist) {
			f.origDist = 0.0f;
			f.normal = glm::cross(ab, ac);
		}

		// Keep normals pointing out from origin
		if (f.origDist < 0) {
			unsigned tmp = f.b;
			f.b = f.c;
			f.c = tmp;
			f.normal = -f.normal;
			f.origDist = -f.origDist;
		}

		faceCount++;
	}

	unsigned indexOf(glm::vec3 v) {
		for (unsigned i = 0; i < vertexCount; ++i)
			if (glm::distance2(vertices[i],v) < FLT_EPSILON*FLT_EPSILON)
				return i;

		return vertexCount;
	}

	void add(glm::vec3 v, glm::vec3 s, unsigned faceIndex) {
		Edge edges[maxPolytopeDim];
		unsigned edgeCount = 0;

		// First, find all faces with normals that face towards this new point
		for (unsigned i = 0; i < faceCount; ++i) {
			if (glm::dot(v - vertices[faces[i].a], (glm::vec3)faces[i].normal) > 0) {

				Edge edge1 = { faces[i].a , faces[i].b };
				Edge edge2 = { faces[i].a , faces[i].c };
				Edge edge3 = { faces[i].b , faces[i].c };

				bool useEdge1 = true;
				bool useEdge2 = true;
				bool useEdge3 = true;

				// If edge exists in edge-list, remove from list
				for (unsigned j = 0; j < edgeCount; ++j) {
					if (edges[j].mirrorEquals(edge1)) {
						// remove edges[j]
						edges[j] = edges[--edgeCount];
						useEdge1 = false;
					}

					if (edges[j].mirrorEquals(edge2)) {
						// remove edges[j]
						edges[j] = edges[--edgeCount];
						useEdge2 = false;
					}

					if (edges[j].mirrorEquals(edge3)) {
						// remove edges[j]
						edges[j] = edges[--edgeCount];
						useEdge3 = false;
					}
				}

				// Add edges
				if (useEdge1) { edges[edgeCount++] = edge1; }
				if (useEdge2) { edges[edgeCount++] = edge2; }
				if (useEdge3) { edges[edgeCount++] = edge3; }

				// Remove face
				faces[i] = faces[--faceCount];
			}
		}
		if (vertexCount < maxPolytopeVert) {
			vertices[vertexCount++] = v;
			supports[vertexCount - 1] = s;
		}
		else {
			throw "Overflow of EPA polytope memory";
		}

		//unsigned a = faces[faceIndex].a;
		//unsigned b = faces[faceIndex].b;
		//unsigned c = faces[faceIndex].c;
		//faces[faceIndex] = faces[--faceCount];
		//addFace(a, b, vertexCount - 1);
		//addFace(a, c, vertexCount - 1);
		//addFace(b, c, vertexCount - 1);
		

		// Add a new triangle for each edge remaining
		for (unsigned i = 0; i < edgeCount; ++i) {
			assert(edges[i].a < vertexCount);
			assert(edges[i].b < vertexCount);
			addFace(edges[i].a, edges[i].b, vertexCount - 1);
		}

		// TODO: If there are any unused vertices, remove them?

	}

	unsigned getMinDistFace() {
		if (faceCount == 0) {
			throw "Cannot find minimally-distant face of empty polytope";
		}

		float minDist = FLT_MAX;
		unsigned minDistFaceIndex = 0;
		for (unsigned i = 0; i < faceCount; ++i) {
			if (faces[i].origDist <= minDist) {
				minDistFaceIndex = i;
				minDist = faces[i].origDist;
			}
		}
		return minDistFaceIndex;
	}
};

void simplifySimplex(GJK::Simplex& S) {
	for (int i = 0; i < S.dim; ++i) {
		for (int j = i + 1; j < S.dim; ++j) {
			if (glm::length2(S.vertices[i] - S.vertices[j]) < FLT_EPSILON * FLT_EPSILON * 4) {
				for (int k = j; k + 1 < S.dim; ++k) {
					S.vertices[k] = S.vertices[k + 1];
				}
				j--;
				S.dim--;
			}
		}
	}
}

bool Collision::solveEPA(GJK::Simplex& S) {

	
	simplifySimplex(S);
	

	constexpr float EPA_epsilon = FLT_EPSILON * FLT_EPSILON * 4;
	glm::vec3 relPos = bodyA->targetTransform.getPosition() - bodyB->targetTransform.getPosition();
	auto supportA = [&](const glm::vec3& dir) {
		if (time == 1.f) {
			glm::quat rot = bodyA->targetTransform.getRotation();
			return rot * shapeA->support(dir * rot);
		}
		else {
			glm::quat rot = glm::slerp(bodyA->currentTransform.getRotation(), bodyA->targetTransform.getRotation(), time);
			return rot * shapeA->support(dir * rot);
		}
	};

	auto supportB = [&](const glm::vec3& dir) {
		if (time == 1.f) {
			glm::quat rot = bodyB->targetTransform.getRotation();
			return rot * shapeB->support(dir * rot);
		}
		else {
			glm::quat rot = glm::slerp(bodyB->currentTransform.getRotation(), bodyB->targetTransform.getRotation(), time);
			return rot * shapeB->support(dir * rot);
		}

	};

	auto support = [&](const glm::vec3& dir) {
		return supportA(dir) - supportB(-dir);
	};

	if (S.dim < 1) {
		return false;
	}

	if (S.dim == 1) {
		//S.vertices[1] = support(S.supports[0]);
		return true;
		
	}

	if (S.dim == 2) {
		return true;
	}

	/*
	if (S.dim == 2) {

		// get vector non-parrallel to collision axis
		glm::vec3 parallel = (S.vertices[0] - S.vertices[1]);
		if (glm::length2(parallel) < EPA_epsilon) {
			parallel = (S.vertices[1] - S.vertices[0]);
		}
		if (glm::length2(parallel) < EPA_epsilon) {
			return false;
		}

		glm::vec3 dir = glm::cross(parallel, { 1, 0, 0 });
		if (glm::length2(dir) < EPA_epsilon) {
			dir = glm::cross(parallel, { 0, 0, 1 });
		}

		S.vertices[2] = support(dir);
		S.dim = 3;
	}

	if (S.dim == 3) {
		glm::vec3 ab = S.vertices[1] - S.vertices[2];
		glm::vec3 ac = S.vertices[0] - S.vertices[2];
		glm::vec3 dir = glm::normalize(glm::cross(ab, ac));
		if (dir != dir) {
			if (glm::length2(ab - ac) < EPA_epsilon) {
				S.vertices[1] = S.vertices[2];

			}
			else {
				if (glm::length2(ab) < EPA_epsilon) {
					S.vertices[1] = S.vertices[2];
				}
				else {
					S.vertices[0] = S.vertices[2];
				}
			}
			S.dim = 2;

			// get vector non-parrallel to collision axis
			glm::vec3 parallel = (S.vertices[0] - S.vertices[1]);
			if (glm::length2(parallel) < EPA_epsilon) {
				parallel = (S.vertices[1] - S.vertices[0]);
			}
			if (glm::length2(parallel) < EPA_epsilon) {
				return false;
			}

			dir = glm::cross(parallel, { 1, 0, 0 });
			if (glm::length2(dir) < EPA_epsilon) {
				dir = glm::cross(parallel, { 0, 0, 1 });
			}

			S.vertices[2] = support(dir);
			S.dim = 3;
			ab = S.vertices[1] - S.vertices[2];
			ac = S.vertices[0] - S.vertices[2];
			dir = glm::normalize(glm::cross(ab, ac));
		}
		assert(dir == dir);
		S.vertices[3] = dir;
		S.dim = 4;
	}
	*/

	// start with GJK simplex containing origin
	Polytope P;
	P.vertexCount = S.dim;
	for (unsigned i = 0; i < S.dim; ++i) {
		P.vertices[i] = support(S.vertices[i]) + relPos;
		P.supports[i] = S.vertices[i];
	}
	if (S.dim >= 3) {
		P.addFace(0, 1, 2);
	}
	if (S.dim == 4) {
		P.addFace(3, 1, 2);
		P.addFace(0, 3, 2);
		P.addFace(0, 1, 3);
	}
	

	for (int i = 0; i < P.vertexCount; ++i) {
		std::cout << "v " << P.vertices[i].x << " " << P.vertices[i].y << " " << P.vertices[i].z << std::endl;
	}
	for (int i = 0; i < P.faceCount; ++i) {
		std::cout << "f " << (P.faces[i].a + 1) << " " << (P.faces[i].b + 1) << " " << (P.faces[i].c + 1) << std::endl;
	}
	std::cout << "----------------------------" << std::endl;

	unsigned closestFace = P.getMinDistFace();
	glm::vec3 supportPoint;
	float supportDist;
	if (P.faces[closestFace].origDist < FLT_EPSILON * FLT_EPSILON) {
		glm::vec3 support1 = support(P.faces[closestFace].normal) + relPos;
		glm::vec3 support2 = support(-P.faces[closestFace].normal) + relPos;
		float support1dist = (float) glm::dot((glm::dvec3) support1, P.faces[closestFace].normal);
		float support2dist = (float) glm::dot((glm::dvec3) support2, -P.faces[closestFace].normal);

		if (support1dist < support2dist) {
			supportPoint = support1;
			supportDist = support1dist;
		}
		else {
			supportPoint = support2;
			supportDist = support2dist;
			P.faces[closestFace].normal = -P.faces[closestFace].normal;
		}
	}
	else {
		supportPoint = support(P.faces[closestFace].normal) + relPos;
		supportDist = (float) glm::dot((glm::dvec3) supportPoint, P.faces[closestFace].normal);
	}
	normal = P.faces[closestFace].normal;
	
	// If square distance is close enough to distance to plane, end
	while (
		supportDist - P.faces[closestFace].origDist > EPA_epsilon
	) {
		//(glm::length2(supportPoint) - (P.faces[closestFace].origDist * P.faces[closestFace].origDist))
		// If P contains support point
		unsigned sIndex = P.indexOf(supportPoint);
		if (sIndex < P.vertexCount) {
			break;
		}

		assert(P.vertexCount >= 4);
		if (EPA_Iterations++ > 10) {
			break;
		}

		// Add the support point to the polytope
		P.add(supportPoint, P.faces[closestFace].normal, closestFace);
		EPA_VertexCount = P.vertexCount;
		EPA_FaceCount = P.faceCount;

		// Calculate new closest face of polytope to origin
		closestFace = P.getMinDistFace();

		// Find support point in the direction of the normal of the closest face
		supportPoint = support(P.faces[closestFace].normal) + relPos;

		// Calculate distance along tested normal
		supportDist = (float) glm::dot((glm::dvec3) supportPoint, P.faces[closestFace].normal);
	}
	
	normal = P.faces[closestFace].normal;

	distance = P.faces[closestFace].origDist;
	assert(distance == distance);

	assert(normal == normal);
	assert(normal != glm::zero<glm::vec3>());

	normal = glm::normalize(normal);
	if (distance > 0.f) {
		if (fabsf(normal.x) > FLT_EPSILON) {
			normal = normal;
		}

		if (fabsf(normal.y) > FLT_EPSILON) {
			normal = normal;
		}
	}
	
	assert(P.faces[closestFace].a < P.vertexCount);
	assert(P.faces[closestFace].b < P.vertexCount);
	assert(P.faces[closestFace].c < P.vertexCount);

	const glm::vec3& a = P.vertices[P.faces[closestFace].a];
	const glm::vec3& b = P.vertices[P.faces[closestFace].b];
	const glm::vec3& c = P.vertices[P.faces[closestFace].c];
	const glm::vec3& a_sv = P.supports[P.faces[closestFace].a];
	const glm::vec3& b_sv = P.supports[P.faces[closestFace].b];
	const glm::vec3& c_sv = P.supports[P.faces[closestFace].c];

	for (int i = 0; i < P.vertexCount; ++i) {
		std::cout << "v " << P.vertices[i].x << " " << P.vertices[i].y << " " << P.vertices[i].z << std::endl;
	}
	for (int i = 0; i < P.faceCount; ++i) {
		std::cout << "f " << (P.faces[i].a+1) << " " << (P.faces[i].b + 1) << " " << (P.faces[i].c + 1) << std::endl;
	}
	
		
	//glm::vec3 a_sa = supportA(a_sv);
	//glm::vec3 a_sb = supportB(-a_sv);
	//glm::vec3 a_sc = support(a_sv) + relPos;
	//glm::vec3 b_sa = supportA(b_sv);
	//glm::vec3 b_sb = supportB(-b_sv);
	//glm::vec3 b_sc = support(b_sv) + relPos;
	//glm::vec3 c_sa = supportA(c_sv);
	//glm::vec3 c_sb = supportB(-c_sv);
	//glm::vec3 c_sc = support(c_sv) + relPos;
	//float dA_s = glm::length2(a_sc - a);
	//float dB_s = glm::length2(b_sc - b);
	//float dC_s = glm::length2(c_sc - c);
	assert(glm::length2(support(a_sv) + relPos - a) < FLT_EPSILON * FLT_EPSILON);
	assert(glm::length2(support(b_sv) + relPos - b) < FLT_EPSILON * FLT_EPSILON);
	assert(glm::length2(support(c_sv) + relPos - c) < FLT_EPSILON * FLT_EPSILON);



	
	// calculate fractional area
	glm::vec3 n = glm::cross(b - a, c - a);


	glm::vec3 aRel = a;
	glm::vec3 bRel = b;
	glm::vec3 cRel = c;
	glm::vec3 bcCross = glm::cross(bRel, cRel);
	glm::vec3 caCross = glm::cross(cRel, aRel);
	glm::vec3 abCross = glm::cross(aRel, bRel);

	float u_abc = glm::dot(bcCross, n);
	float v_abc = glm::dot(caCross, n);
	float w_abc = glm::dot(abCross, n);
	
	glm::vec3 A_va = supportA(a_sv);
	glm::vec3 A_vb = supportA(b_sv);
	glm::vec3 A_vc = supportA(c_sv);

	glm::vec3 B_va = supportB(-a_sv);
	glm::vec3 B_vb = supportB(-b_sv);
	glm::vec3 B_vc = supportB(-c_sv);
	
	glm::vec3 closestPoint = P.faces[closestFace].origDist * (glm::vec3) P.faces[closestFace].normal;
	glm::vec3 recalcPoint = (u_abc * (A_va - B_va + relPos) + v_abc * (A_vb - B_vb + relPos) + w_abc * (A_vc - B_vc + relPos)) * (1.f/glm::length2(n));

	// x*a + y*b + z*c = closestPoint
	//


	glm::vec3 hitspotA_nonNorm = (
		u_abc * A_va +
		v_abc * A_vb +
		w_abc * A_vc
	);

	glm::vec3 hitspotB_nonNorm = (
		u_abc * B_va +
		v_abc * B_vb +
		w_abc * B_vc
	);

	float area1 = u_abc + v_abc + w_abc;
	float area2 = glm::length2(n);

	float u_norm = u_abc / glm::length2(n);
	float v_norm = v_abc / glm::length2(n);
	float w_norm = w_abc / glm::length2(n);

	double areaA = sqrt(glm::length2(glm::cross(A_vb - A_va, A_vc - A_va)));
	double areaB = sqrt(glm::length2(glm::cross(B_vb - B_va, B_vc - B_va)));

	double invlen2 = (1.0 / glm::length2(n));

	glm::vec3 hitspotRelA = ((glm::dvec3) hitspotA_nonNorm) * invlen2;
	glm::vec3 hitspotRelB = ((glm::dvec3) hitspotB_nonNorm) * invlen2;
	glm::vec3 hitspotA = hitspotRelA + bodyA->targetTransform.getPosition();
	glm::vec3 hitspotB = hitspotRelB + bodyB->targetTransform.getPosition();
	glm::vec3 hitspotC = hitspotA * (float)(areaB / (areaA + areaB)) + hitspotB * (float)(areaA / (areaA + areaB));
	hitSpot = hitspotC; //(
	//	(hitspotA_nonNorm + hitspotB_nonNorm) * (1 / glm::length2(n)) +
	//	bodyA->targetTransform.getPosition() + bodyB->targetTransform.getPosition()
	//) * 0.5f;
	
	/*
	const glm::vec3& A_pos = bodyA->targetTransform.getPosition();
	const glm::vec3& B_pos = bodyB->targetTransform.getPosition();
	glm::vec3 A_va = supportA(a_sv) + A_pos;
	glm::vec3 A_vb = supportA(b_sv) + A_pos;
	glm::vec3 A_vc = supportA(c_sv) + A_pos;
	glm::vec3 B_va = supportB(-a_sv) + B_pos;
	glm::vec3 B_vb = supportB(-b_sv) + B_pos;
	glm::vec3 B_vc = supportB(-c_sv) + B_pos;

	glm::vec3 A_3a = (A_va - glm::dot(A_va, normal) * normal);
	glm::vec2 A_2a = { A_3a.x, A_3a.y };
	glm::vec3 A_3b = (A_vb - glm::dot(A_vb, normal) * normal);
	glm::vec2 A_2b = { A_3b.x, A_3b.y };
	glm::vec3 A_3c = (A_vc - glm::dot(A_vc, normal) * normal);
	glm::vec2 A_2c = { A_3c.x, A_3c.y };

	glm::vec3 B_3a = (B_va - glm::dot(B_va, normal) * normal);
	glm::vec2 B_2a = { B_3a.x, B_3a.y };
	glm::vec3 B_3b = (B_vb - glm::dot(B_vb, normal) * normal);
	glm::vec2 B_2b = { B_3b.x, B_3b.y };
	glm::vec3 B_3c = (A_vc - glm::dot(B_vc, normal) * normal);
	glm::vec2 B_2c = { B_3c.x, B_3c.y };

	auto inTriangle = [](const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& p) {
		float s = a.y * c.x - a.x * c.y + (c.y - a.y) * p.x + (a.x - c.x) * p.y;
		float t = a.x * b.y - a.y * b.x + (a.y - b.y) * p.x + (b.x - a.x) * p.y;

		if ((s < 0) != (t < 0))
			return false;

		float A = -b.y * c.x + a.y * (c.x - b.x) + a.x * (b.y - c.y) + b.x * c.y;
		return A < 0 ? (s <= 0 && s + t >= A) : (s >= 0 && s + t <= A);
	};

	auto lineIntersect = [](const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2, glm::vec2& p) {

	};

	bool A1inB = inTriangle(B_2a, B_2b, B_2c, A_2a);
	bool A2inB = inTriangle(B_2a, B_2b, B_2c, A_2b);
	bool A3inB = inTriangle(B_2a, B_2b, B_2c, A_2c);
	bool B1inA = inTriangle(A_2a, A_2b, A_2c, B_2a);
	bool B2inA = inTriangle(A_2a, A_2b, A_2c, B_2b);
	bool B3inA = inTriangle(A_2a, A_2b, A_2c, B_2c);


	if (A1inB && A2inB && A3inB) {
		// center of A
	}
	else if (B1inA && B2inA && B3inA) {
		// center of B
	}
	else if (A1inB) {
		if (A2inB) {
			// line A1A2 in B ; at most 1 vertice of B can be in A
			if (B1inA) {
				// find intersection of A1A3 and B1B2
				// find intersection of A1A3 and B1B3
				// find intersection of A2A3 and B1B2
				// find intersection of A2A3 and B1B3
			}
			else if (B2inA) {
				// find intersection of A1A3 and B1B2
				// find intersection of A1A3 and B2B3
				// find intersection of A2A3 and B1B2
				// find intersection of A2A3 and B2B3
			}
			else if (B3inA) {
				// find intersection of A1A3 and B2B3
				// find intersection of A1A3 and B1B3
				// find intersection of A2A3 and B2B3
				// find intersection of A2A3 and B1B3
			}
			else {
				// find intersection of A1A3 and B1B2
				// find intersection of A1A3 and B1B3
				// find intersection of A1A3 and B1B3
				// find intersection of A2A3 and B1B2
				// find intersection of A2A3 and B1B3
				// find intersection of A2A3 and B1B3
			}

		}
		else {
			if (A3inB) {
				// line A1A3 in B ; at most 1 vertice of B can be in A
				if (B1inA) {
					// find intersection of A1A2 and B1B2
					// find intersection of A1A2 and B1B3
					// find intersection of A2A3 and B1B2
					// find intersection of A2A3 and B1B3
				}
				else if (B2inA) {
					// find intersection of A1A2 and B1B2
					// find intersection of A1A2 and B2B3
					// find intersection of A2A3 and B1B2
					// find intersection of A2A3 and B2B3
				}
				else if (B3inA) {
					// find intersection of A1A2 and B2B3
					// find intersection of A1A2 and B1B3
					// find intersection of A2A3 and B2B3
					// find intersection of A2A3 and B1B3
				}
				else {
					// find intersection of A1A2 and B1B2
					// find intersection of A1A2 and B1B3
					// find intersection of A1A2 and B1B3
					// find intersection of A2A3 and B1B2
					// find intersection of A2A3 and B1B3
					// find intersection of A2A3 and B1B3
				}
			}
			else {
				// only A1 is contained in B; up to 2 points in B can be in A
			}
		}
	}
	else {
		if (A2inB) {
			if (A3inB) {
				// line A2A3 in B ; at most 1 vertice of B can be in A
				if (B1inA) {
					// find intersection of A1A2 and B1B2
					// find intersection of A1A2 and B1B3
					// find intersection of A1A3 and B1B2
					// find intersection of A1A3 and B1B3
				}
				else if (B2inA) {
					// find intersection of A1A2 and B1B2
					// find intersection of A1A2 and B2B3
					// find intersection of A1A3 and B1B2
					// find intersection of A1A3 and B2B3
				}
				else if (B3inA) {
					// find intersection of A1A2 and B2B3
					// find intersection of A1A2 and B1B3
					// find intersection of A1A3 and B2B3
					// find intersection of A1A3 and B1B3
				}
				else {
					// find intersection of A1A2 and B1B2
					// find intersection of A1A2 and B1B3
					// find intersection of A1A2 and B1B3
					// find intersection of A1A3 and B1B2
					// find intersection of A1A3 and B1B3
					// find intersection of A1A3 and B1B3
				}
			}
			else {
				// only A2 is contained in B; up to 2 points in B can be in A
			}
		}
		else {
			if (A3inB) {
				// only A2 is contained in B; up to 2 points in B can be in A
			}
			else {
				// take average of all points
			}
		}
	}

	*/
	assert(hitSpot == hitSpot);

	return true;
}

// WARNING: Unfinished
void Collision::resolveSampledCollisionDetails() {
	glm::vec3 relPos = bodyA->targetTransform.getPosition() - bodyB->targetTransform.getPosition();
	auto supportA = [&](const glm::vec3& dir) {
		if (time == 1.f) {
			glm::quat rot = bodyA->targetTransform.getRotation();
			return rot * shapeA->support(dir * rot);
		}
		else {
			glm::quat rot = glm::slerp(bodyA->currentTransform.getRotation(), bodyA->targetTransform.getRotation(), time);
			return rot * shapeA->support(dir * rot);
		}
	};

	auto supportB = [&](const glm::vec3& dir) {
		if (time == 1.f) {
			glm::quat rot = bodyB->targetTransform.getRotation();
			return rot * shapeB->support(dir * rot);
		}
		else {
			glm::quat rot = glm::slerp(bodyB->currentTransform.getRotation(), bodyB->targetTransform.getRotation(), time);
			return rot * shapeB->support(dir * rot);
		}

	};

	auto support = [&](const glm::vec3& dir) {
		return supportA(dir) - supportB(-dir);
	};

	unsigned numSamples = 100;
	constexpr float phi = glm::pi<float>() * (3.f - glm::root_five<float>());

	// The closest point to the origin along the CSO
	glm::vec3 minPenVec = glm::zero<glm::vec3>();

	// The direction sampled which generated the closest point
	glm::vec3 minSampleVec = glm::zero<glm::vec3>();

	// The depth of the penetration
	float minPenetration = FLT_MAX;

	for (unsigned i = 0; i < numSamples; ++i) {
		float y = 1 - (i / (float)(numSamples - 1)) * 2;
		float radius = sqrtf(1.f - y * y);
		float theta = phi * i;

		glm::vec3 sampleVec = {
			cosf(theta) * radius,
			y,
			sinf(theta) * radius
		};

		glm::vec3 penetration = support(sampleVec) + relPos;
		float penetrationDepth = glm::length2(penetration);
		if (penetrationDepth < minPenetration) {
			minPenetration = penetrationDepth;
			minSampleVec = sampleVec;
			minPenVec = penetration;
		}
	}


	//normal = glm::normalize(minPenetrationVec1);
	//distance = minPenetration;
	//glm::vec3 hitspotA = minPenetrationVec1;
}

bool Collision::handleIntersect(glm::vec3 initialAxis, bool doRaycast) {
	initialAxis = glm::normalize(initialAxis);
	assert(initialAxis.length() > FLT_EPSILON);
	GJK::Simplex S;
		
	if (doRaycast) {
		float t = GJK_RayIntersect(initialAxis, S);
		if (t >= 1.0f || t <= 0.0f) {
			return false;
		}

		if (glm::dot(initialAxis, normal) > FLT_EPSILON) { return false; }

		return solveEPA(S);
	}
	else {
		if (GJK_Distance(initialAxis, S, 1.f) > 0) {
			return false;
		}

		time = 1.f;
		return solveEPA(S);
	}
}

// returns negative 1 if overlapping
float Collision::GJK_Distance(glm::vec3 initialAxis, GJK::Simplex& S, float scale) {

	glm::vec3 relPos = bodyA->targetTransform.getPosition() - bodyB->targetTransform.getPosition();
	auto supportA = [&](const glm::vec3& dir) {
		const glm::quat& rot = bodyA->targetTransform.getRotation();
		return rot * shapeA->support(dir * rot);
	};
	auto supportB = [&](const glm::vec3& dir) {
		const glm::quat& rot = bodyB->targetTransform.getRotation();
		return rot * shapeB->support(dir * rot);
	};

	auto support = [&](const glm::vec3& dir) {
		return supportA(dir) - supportB(-dir);
	};

	if (glm::length2(initialAxis) < FLT_EPSILON * FLT_EPSILON) {
		initialAxis = Vector::down;
	} else {
		initialAxis = glm::normalize(initialAxis);
	}

	const int infiniteLoopWatchdogCount = 70;
	glm::vec3 v = -(support(initialAxis) * scale + relPos);
	S.vertices[0] = -v;
	S.dim = 1;
	glm::vec3 w = v;
	glm::vec3 w_old = { 0,0,0 };
	
	while (glm::length2(v) > FLT_EPSILON * FLT_EPSILON * 2) {
		// Watchdog timer
		if (++GJK_Iterations > infiniteLoopWatchdogCount) {
			GJK_Iterations = GJK_Iterations;
		}
		
		glm::vec3 v_n = glm::normalize(v);
		glm::vec3 w = support(v_n) * scale + relPos;
		if (w == w_old) {
			return glm::length2(v);
		}
		else {
			w_old = w;
		}

		if (glm::dot(v, w) < 0) {
			return glm::length2(v);
		}
		
		S = GJK::Simplex::Union(S, w);
		GJK::doSimplex(S, v);
		v = -v;
	}

	return -1.f;
}

float Collision::GJK_RayIntersect(glm::vec3 initialAxis, GJK::Simplex& S) {
	auto support = [&](const glm::vec3& dir, float t) {
		//return shapeA->support(dir) - shapeB->support(-dir);
		glm::quat rotationA = glm::slerp(bodyA->currentTransform.getRotation(), bodyA->targetTransform.getRotation(), t);
		glm::quat rotationB = glm::slerp(bodyB->currentTransform.getRotation(), bodyB->targetTransform.getRotation(), t);

		glm::vec3 supportA = rotationA * shapeA->support(dir * rotationA);
		glm::vec3 supportB = rotationB * shapeB->support(-dir * rotationB);

		return supportA - supportB;
	};

	lastSimplex = S;

	float eps_tol = FLT_EPSILON * 2 * S.maxDist(hitSpot);

	glm::vec3 s = bodyB->currentTransform.getPosition() - bodyA->currentTransform.getPosition();
	glm::vec3 r = bodyB->targetTransform.getPosition() - bodyA->targetTransform.getPosition() - s;

	glm::vec3 arbitraryPointInC = support(initialAxis, 0);
	hitSpot = s;
	normal = { 0, 0, 0 };
	bool normSet = false;
	glm::vec3 v = hitSpot - arbitraryPointInC;
	if (glm::dot(v, v) <= eps_tol) {
		glm::vec3 tan = glm::cross(initialAxis, { 1,0,0 });
		if (glm::length2(tan) < FLT_EPSILON * FLT_EPSILON * 4) {
			tan = glm::cross(initialAxis, { 0,1,0 });
		}
		arbitraryPointInC = support(tan, 0);
		v = hitSpot - arbitraryPointInC;
	}


	glm::vec3 v_old = v;
	time = 0.0f;
	float time_old = 0.0f;
	const int infiniteLoopWatchdogCount = 100;


	glm::vec3 p = { 0, 0, 0 };
	glm::vec3 p_old = support(glm::normalize(v), time);
	glm::vec3 v_record = {0,0,0};
	GJK::Simplex simplexRecord1;
	GJK::Simplex simplexRecord2;
	hitSpot = s + time * r;

	while (glm::dot(v, v) > eps_tol) {
		if (++GJK_Iterations > infiniteLoopWatchdogCount) {
			GJK_Iterations = GJK_Iterations;
		}

		p_old = p;
		p = support(glm::normalize(v), time);
		glm::vec3 w = hitSpot - p;

		if (glm::dot(v, w) > 0.0f) {
			if (glm::dot(v, r) >= 0.0f) {
				return 1.0f;
			}
			else {
				time -= glm::dot(v, w) / glm::dot(v, r);

				if (time > 1.0f) {
					return 1.0f;
				}

				hitSpot = s + time * r;
				normal = v;
				//normal = v_best;
				normSet = true;
			}
		}

		
		
		
		

		// Check for 
		S = GJK::Simplex::Union(S, p);

		float maxLen = 0.0f;
		for (int k = 0; k < S.dim; ++k) {
			float len = glm::length2(hitSpot - S.vertices[k]);
			if (len > maxLen) {
				maxLen = len;
			}
		}
		if (glm::length2(v) <= FLT_EPSILON * 2 * maxLen) {
			break;
		}

		S = GJK::Simplex::Conv(hitSpot, S);
		
		v_old = v;
		simplexRecord2 = S;
		GJK::doSimplex(S, v);
		v_record = v;
		simplexRecord1 = S;
		//eps_tol = glm::max(FLT_EPSILON * FLT_EPSILON * 2 * S.maxDist(hitSpot), FLT_EPSILON * FLT_EPSILON);
		S = GJK::Simplex::Conv(hitSpot, S);
		lastSimplex = S;
	}
	/*
	if (!normSet) {
		if (glm::length2(v) < FLT_EPSILON * FLT_EPSILON * 4) {
			return 0;
		}
		p = support(glm::normalize(v), time);
		glm::vec3 w = hitSpot - p;

		//float f1 = glm::dot(v, w);
		//float f2 = glm::dot(v, r);

		//glm::vec3 v1 = s - arbitraryPointInC;

		normal = v;
		time -= glm::dot(v, w) / glm::dot(v, r);
	}

	if (time <= 0) {
		return time;
	}*/

	//for (unsigned i = 0; i < S.dim; ++i) {
	//	S.vertices[i]
	//}
	if (S.dim == 1) {

	}
	else {
		S = GJK::Simplex::Conv(hitSpot, S);
	}

	return time;
}

/*
* https://dl-acm-org.offcampus.lib.washington.edu/doi/pdf/10.1145/3083724
*/