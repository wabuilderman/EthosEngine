#pragma once
#include "PhysicsShape.hpp"
#include <exception>
#include <chrono>
#include <iostream>
#include "GJK.hpp"
class PhysicsBody;
struct Collision {
	const PhysicsBody* bodyA;
	const PhysicsBody* bodyB;
	const PhysicsShape* shapeA;
	const PhysicsShape* shapeB;
	
	float time = 0.f;
	float distance = 0.f;
	glm::vec3 normal = {0, 0, 0};
	glm::vec3 hitSpot = { 0, 0, 0 };
	GJK::Simplex lastSimplex;

	unsigned GJK_Iterations = 0;
	unsigned EPA_Iterations = 0;
	unsigned EPA_FaceCount = 0;
	unsigned EPA_VertexCount = 0;

	float GJK_RayIntersect(glm::vec3 deltaPos, GJK::Simplex& S);
	float GJK_Distance(glm::vec3 deltaPos, GJK::Simplex& S, float scale);
	bool solveEPA(GJK::Simplex& S);
	void resolveSampledCollisionDetails();
	glm::vec3 CSO_Support_Transformed(const glm::vec3& dir);

	glm::vec3 A_Support_Transformed(const glm::vec3& dir);
	glm::vec3 B_Support_Transformed(const glm::vec3& dir);

	bool handleIntersect(glm::vec3 initialAxis, bool doRaycast);

	Collision(const PhysicsBody* bodyA, const PhysicsBody* bodyB, const PhysicsShape* shapeA, const PhysicsShape* shapeB) : bodyA(bodyA), bodyB(bodyB), shapeA(shapeA), shapeB(shapeB) {}
};