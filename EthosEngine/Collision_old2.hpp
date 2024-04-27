#pragma once
#include "PhysicsShape.hpp"
#include <exception>
#include <chrono>

struct Collision {
	const PhysicsShape* shapeA;
	const PhysicsShape* shapeB;

	float time = 0.0f;
	glm::vec3 normal = {0, 0, 0};
	glm::vec3 hitSpot = { 0, 0, 0 };

	struct Simplex {
		glm::vec3 vertices[4];
		unsigned dim;
	};

	glm::vec3 GJK_Support(const glm::vec3& dir) {
		return shapeA->supportCurrent(dir) - shapeB->supportCurrent(-dir);
	}

	static bool GJK_NormalHelper(Simplex& Q, glm::vec3& n) {
		// OLD DESCRIPTION - TODO: UPDATE
		// distance from plane defined by normal n and point P, and point Q
		// d = ((PQ) dot n)/||n||
		// Therefore, normalize D (our normal in the direction of Q, the origin)
		// And dot with the negative position vector of a point in the plane
		// For bonus points, if d < epsilon^2, then the origin is contained along this simplex


		if (glm::length2(n) < FLT_EPSILON * FLT_EPSILON) {
			switch (Q.dim) {
			case 2: {
				n = Q.vertices[0];
			}break;
			case 3: {
				n = Q.vertices[0];
			}break;
			default:
				return true;
			}
			return true;
		}
		else {
			n = glm::normalize(n);
			float d = glm::dot(Q.vertices[0], n);

			if (fabs(d) < FLT_EPSILON * FLT_EPSILON) {
				return true;
			}

			n = d * n;
			return false;
		}
	}

	static bool GJK_DoSimplex(Simplex& Q, glm::vec3& D) {
		switch (Q.dim) {
		case 1: {
			D = -Q.vertices[0];
			return false;
		}
		// Line Case
		case 2: {
			glm::vec3 ab = Q.vertices[0] - Q.vertices[1];
			glm::vec3 ao = -Q.vertices[1];

			// OPTIONAL -----------------------------
			if (glm::dot(ab, ao) < 0) {
				D = ao;
				Q.vertices[0] = Q.vertices[1];
				Q.dim = 1;
				return false;
			}
			// --------------------------------------

			D = glm::cross(glm::cross(ab, ao), ab);
			return GJK_NormalHelper(Q, D);
		}
		// Triangle Case
		case 3: {
			glm::vec3 ab = Q.vertices[1] - Q.vertices[2];
			glm::vec3 ac = Q.vertices[0] - Q.vertices[2];
			glm::vec3 ao = -Q.vertices[2];
			glm::vec3 abc = glm::cross(ab, ac);

			// Plane-test for plane containing the line AC, facing the origin
			if (glm::dot(glm::cross(abc, ac), ao) > 0) {
				// Line AC is closest
				D = glm::cross(glm::cross(ac, ao), ac);
				Q.vertices[1] = Q.vertices[2];
				Q.dim = 2;
			}
			else {
				// Plane-test for plane containing the line AB, facing the origin
				if (glm::dot(glm::cross(ab, abc), ao) > 0) {
					// Line AB is closest
					D = glm::cross(glm::cross(ab, ao), ab);
					Q.vertices[0] = Q.vertices[1];
					Q.vertices[1] = Q.vertices[2];
					Q.dim = 2;
				}
				else {
					// Triangle ABC is closest

					// Correct normal
					if (glm::dot(abc, Q.vertices[0]) < 0) {
						D = abc;
					}
					else {
						D = -abc;
						glm::vec3 tmp = Q.vertices[0];
						Q.vertices[0] = Q.vertices[1];
						Q.vertices[1] = tmp;
					}
				}
			}

			return GJK_NormalHelper(Q, D);
		}
		// Tetrahedron Case
		default: {
			glm::vec3 ab = Q.vertices[2] - Q.vertices[3];
			glm::vec3 ac = Q.vertices[1] - Q.vertices[3];
			glm::vec3 ad = Q.vertices[0] - Q.vertices[3];
			glm::vec3 ao = -Q.vertices[3];

			glm::vec3 abc = glm::cross(ab, ac);
			glm::vec3 abd = glm::cross(ad, ab);
			glm::vec3 acd = glm::cross(ac, ad);

			/*// OPTIONAL -----------------------------
			if (glm::dot(ab, ao) < 0 && glm::dot(ac, ao) < 0 && glm::dot(ad, ao) < 0) {
				D = ao;
				Q.vertices[0] = Q.vertices[3];
				Q.dim = 1;
				return false;
			}
			// --------------------------------------*/

			if (glm::dot(abc, ao) > 0) {
				if (glm::dot(abd, ao) > 0) {
					if (glm::dot(glm::cross(ab, abc), ao) > 0) {
						if (glm::dot(glm::cross(ab, abd), ao) > 0) {
							// region AB
							D = glm::cross(glm::cross(ab, ao), ab);
							Q.vertices[0] = Q.vertices[2];
							Q.vertices[1] = Q.vertices[3];
							Q.dim = 2;

							return GJK_NormalHelper(Q, D);
						}
						else {
							// region ABD
							D = abd;
							Q.vertices[1] = Q.vertices[2];
							Q.vertices[2] = Q.vertices[3];
							Q.dim = 3;

							return GJK_NormalHelper(Q, D);
						}
					}
					else {
						// region ABC
						D = abc;
						Q.vertices[0] = Q.vertices[1];
						Q.vertices[1] = Q.vertices[2];
						Q.vertices[2] = Q.vertices[3];
						Q.dim = 3;

						return GJK_NormalHelper(Q, D);
					}
				}
				else {
					if (glm::dot(acd, ao) > 0) {
						if (glm::dot(glm::cross(ac, abc), ao) > 0) {
							if (glm::dot(glm::cross(ac, acd), ao) > 0) {
								// region AC
								D = glm::cross(glm::cross(ac, ao), ac);
								Q.vertices[0] = Q.vertices[1];
								Q.vertices[1] = Q.vertices[3];
								Q.dim = 2;

								return GJK_NormalHelper(Q, D);
							}
							else {
								// region ACD
								D = acd;
								Q.vertices[2] = Q.vertices[3];
								Q.dim = 3;

								return GJK_NormalHelper(Q, D);
							}
						}
						else {
							// region ABC
							D = abc;
							Q.vertices[0] = Q.vertices[1];
							Q.vertices[1] = Q.vertices[2];
							Q.vertices[2] = Q.vertices[3];
							Q.dim = 3;

							return GJK_NormalHelper(Q, D);
						}
					}
					else {
						if (glm::dot(glm::cross(ab, abc), ao) > 0) {
							// region AB
							D = glm::cross(glm::cross(ab, ao), ab);
							Q.vertices[0] = Q.vertices[2];
							Q.vertices[1] = Q.vertices[3];
							Q.dim = 2;

							return GJK_NormalHelper(Q, D);
						}
						else {
							if (glm::dot(glm::cross(ac, abc), ao) > 0) {
								// region AC
								D = glm::cross(glm::cross(ac, ao), ac);
								Q.vertices[0] = Q.vertices[1];
								Q.vertices[1] = Q.vertices[3];
								Q.dim = 2;

								return GJK_NormalHelper(Q, D);
							}
							else {
								// region ABC
								D = abc;
								Q.vertices[0] = Q.vertices[1];
								Q.vertices[1] = Q.vertices[2];
								Q.vertices[2] = Q.vertices[3];
								Q.dim = 3;

								return GJK_NormalHelper(Q, D);
							}
						}
					}
				}
			}
			else {
				if (glm::dot(abd, ao) > 0) {
					if (glm::dot(acd, ao) > 0) {
						if (glm::dot(glm::cross(ad, acd), ao) > 0) {
							if (glm::dot(glm::cross(ad, abd), ao) > 0) {
								// region AD
								D = glm::cross(glm::cross(ad, ao), ad);
								Q.vertices[1] = Q.vertices[3];
								Q.dim = 2;

								return GJK_NormalHelper(Q, D);
							}
							else {
								// region ABD
								D = abd;
								Q.vertices[1] = Q.vertices[2];
								Q.vertices[2] = Q.vertices[3];
								Q.dim = 3;

								return GJK_NormalHelper(Q, D);
							}
						}
						else {
							// region ACD
							D = acd;
							Q.vertices[2] = Q.vertices[3];
							Q.dim = 3;

							return GJK_NormalHelper(Q, D);
						}
					}
					else {
						if (glm::dot(glm::cross(ab, abd), ao) > 0) {
							// region AB
							D = glm::cross(glm::cross(ab, ao), ab);
							Q.vertices[0] = Q.vertices[2];
							Q.vertices[1] = Q.vertices[3];
							Q.dim = 2;

							return GJK_NormalHelper(Q, D);
						}
						else {
							if (glm::dot(glm::cross(ad, abd), ao) > 0) {
								// region AD
								D = glm::cross(glm::cross(ad, ao), ad);
								Q.vertices[1] = Q.vertices[3];
								Q.dim = 2;

								return GJK_NormalHelper(Q, D);
							}
							else {
								// region ABD
								D = abd;
								Q.vertices[1] = Q.vertices[2];
								Q.vertices[2] = Q.vertices[3];
								Q.dim = 3;

								return GJK_NormalHelper(Q, D);
							}
						}
					}
				}
				else {
					if (glm::dot(acd, ao) > 0) {
						if (glm::dot(glm::cross(ac, acd), ao) > 0) {
							// region AC
							D = glm::cross(glm::cross(ac, ao), ac);
							Q.vertices[0] = Q.vertices[1];
							Q.vertices[1] = Q.vertices[3];
							Q.dim = 2;

							return GJK_NormalHelper(Q, D);
						}
						else {
							if (glm::dot(glm::cross(ad, acd), ao) > 0) {
								// region AD
								D = glm::cross(glm::cross(ad, ao), ad);
								Q.vertices[1] = Q.vertices[3];
								Q.dim = 2;

								return GJK_NormalHelper(Q, D);
							}
							else {
								// region ACD
								D = acd;
								Q.vertices[2] = Q.vertices[3];
								Q.dim = 3;

								return GJK_NormalHelper(Q, D);
							}
						}
					}
					else {
						// region I
						return true;
					}
				}
			}
		}
		}
	}

	glm::vec3 GJK_raw_findClosestPoint(glm::vec3 x) {
		constexpr float eps2 = FLT_EPSILON * FLT_EPSILON;

		// start with any point
		glm::vec3 v = GJK_Support({ 0, 0, 1 });
		glm::vec3 w = GJK_Support(-v);

		Simplex P = {
			.vertices = {
				{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }
			},
			.dim = 0
		};

		unsigned iter = 0;

		while (fabs((glm::dot(v, v) - glm::dot(v, w))) > eps2) {
			// Adds new point to simplex, which increases it's dimensionality
			P.vertices[P.dim++] = w;

			if (GJK_DoSimplex(P, v)) {
				return v;
			}

			w = GJK_Support(-v);

			// Check if w is colinear with the line AB
			if (P.dim == 2) {
				if (glm::length2(glm::cross(w - P.vertices[1], P.vertices[0] - P.vertices[1])) < eps2) {
					return v;
				}
			}

			if (P.dim == 3) {
				if (fabs(glm::dot(w - P.vertices[2], v)) < eps2) {
					return v;
				}
			}

			// Check if w is coplanar with the plane ABC
			//if (P.dim == 3) {
			//	if ((glm::dot(w, v) - glm::dot(v,v)) < eps2) {
			//		return v;
			//	}
			//}


			if (iter++ > 32) {
				std::cout << "stuck" << std::endl;
			}
		}
		return v;
	}

	// http://dtecta.com/papers/jgt04raycast.pdf
	// https://arxiv.org/ftp/arxiv/papers/1505/1505.07873.pdf
	// https://caseymuratori.com/blog_0003

	bool GJK_Intersect(glm::vec3 relDeltaPos) {
		constexpr float eps2 = FLT_EPSILON * FLT_EPSILON;
		glm::vec3 s = glm::vec3(0, 0, 0);
		glm::vec3 r = relDeltaPos;
		//glm::vec3 ray = -relDeltaPos;

		//time = 0.0f;
		//glm::vec3 c = source;
		//normal = { 0, 0, 0 };
		//hitSpot = GJK_raw_findClosestPoint(c);
		//return true;

		float lamda = 0.0f;
		glm::vec3 x = s;
		glm::vec3 n = glm::vec3(0, 0, 0);
		glm::vec3 v = x - GJK_Support({ 0, 0, 1 });
		Simplex P = {
			.vertices = {
				{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }
			},
			.dim = 0
		};
		while (glm::dot(v, v) > eps2) {
			glm::vec3 p = GJK_Support(v);
			glm::vec3 w = x - p;
			if (glm::dot(v, w) > 0) {
				if (glm::dot(v, r) >= 0) {
					return false;
				}
				else {
					lamda = lamda - (glm::dot(v, w) / glm::dot(v, r));
					x = s + lamda * r;
					n = v;
				}
			}

			//if (P.dim == 4) { return false; }
			//Simplex Y = P.Union(p);
			//v = pointClosestToOrigin({x} - Y);
			//P = "Smallest X subset of Y, such that v is an element of conv({x} - Y)"

		}
		return true;


		/*while (glm::length2(c - hitSpot) > 0.01f) {
			normal = hitSpot - c;
			if (glm::dot(normal, ray) >= 0) {
				return false;
			}
			else {
				time = time - (glm::dot(normal, normal) / glm::dot(normal, ray));
				hitSpot = source + time * ray;
				c = GJK_raw_findClosestPoint(hitSpot);
			}
		}
		return true;*/

		/*

		if (glm::length2(ray) < eps2) {
			return false;
		}

		glm::vec3 v = hitSpot - GJK_Support({ 0, 0, 1 });

		Simplex P = {
			.vertices = {
				{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }
			},
			.dim = 0
		};

		
		unsigned iter = 0;

		while (glm::dot(v, v) > eps2) {

			glm::vec3 p = GJK_Support(v);
			glm::vec3 w = hitSpot - p;

			// Essentially, checks if A is on the other side of the origin,
			// which must be true in order for the shapes to intersect.

			if (glm::dot(v, w) > 0) {
				if (glm::dot(v, ray) >= 0) {
					return false;
				}
				else {
					time = time - (glm::dot(v, w) / glm::dot(v, ray));
					hitSpot = source + time * ray;
					normal = v;
				}
			}

			if (P.dim == 4) {
				return false;
			}

			// Adds new point to simplex, which increases it's dimensionality
			P.vertices[P.dim] = p;
			P.dim++;

			for (unsigned i = 0; i < P.dim; ++i) {
				P.vertices[i] = hitSpot - P.vertices[i];
			}

			v = GJK_DoSimplexFoolproof(P);

			if (iter++ > 40) {
				std::cout << "stuck!" << std::endl;
			}

			if (isnan(v.x) || isnan(v.y) || isnan(v.z)) {
				return false;
			}

			if (glm::dot(v, v) < eps2) {
				return true;
			}
		}

		return true;*/
	}

	Collision(const PhysicsShape* shapeA, const PhysicsShape* shapeB) : shapeA(shapeA), shapeB(shapeB) {
	}
};