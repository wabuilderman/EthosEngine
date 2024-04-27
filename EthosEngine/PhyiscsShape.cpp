#include "PhysicsShape.hpp"

float cosineSimilarity(glm::vec3 a, glm::vec3 b) {
	return glm::dot(a, b) / (glm::length(a) * glm::length(b));
}

// Initialization
SpherePhysicsShape::SpherePhysicsShape(float radius) : radius(radius) {
	interiaTensor = glm::identity<glm::mat3x3>() * (radius * radius * 2.0f / 5.0f);
}

// GJK suppport function
glm::vec3 SpherePhysicsShape::support(glm::vec3 dir) const {
	// Normalize direction Offset by origin of sphere
	return radius * dir;
}
glm::vec3 SpherePhysicsShape::supportNormal(glm::vec3 dir) const {
	return dir;
	// for deformed sphere: glm::normalize(dir * dim);
}

// Initialization
BoxPhysicsShape::BoxPhysicsShape(glm::vec3 dim) : dim(dim) {
	interiaTensor = {
		((dim.y * dim.y + dim.z * dim.z) / 12.0f), 0, 0,
		0, ((dim.x * dim.x + dim.z * dim.z) / 12.0f), 0,
		0, 0, ((dim.y * dim.y + dim.x * dim.x) / 12.0f)
	};
}

// GJK suppport function
glm::vec3 BoxPhysicsShape::support(glm::vec3 dir) const {
	dir.x = (abs(dir.x) < FLT_EPSILON ? 0.0f : ((dir.x > 0.0f) ? (.5f) : (-.5f)));
	dir.y = (abs(dir.y) < FLT_EPSILON ? 0.0f : ((dir.y > 0.0f) ? (.5f) : (-.5f)));
	dir.z = (abs(dir.z) < FLT_EPSILON ? 0.0f : ((dir.z > 0.0f) ? (.5f) : (-.5f)));
	//dir.x = dir.x > 0.f ? .5f : -.5f;
	//dir.y = dir.y > 0.f ? .5f : -.5f;
	//dir.z = dir.z > 0.f ? .5f : -.5f;
	return dim * dir;
}

glm::vec3 BoxPhysicsShape::supportNormal(glm::vec3 dir) const {
	//dir.x = (abs(dir.x) < FLT_EPSILON ? 0.0f : ((dir.x > 0.0f) ? (.5f) : (-.5f)));
	//dir.y = (abs(dir.y) < FLT_EPSILON ? 0.0f : ((dir.y > 0.0f) ? (.5f) : (-.5f)));
	//dir.z = (abs(dir.z) < FLT_EPSILON ? 0.0f : ((dir.z > 0.0f) ? (.5f) : (-.5f)));

	dir = glm::normalize(dir * dim);

	glm::vec3 bestResult = { 0, 0, 0 };
	float bestSimilarity = FLT_MIN;

	// test face-normals
	float xTest = cosineSimilarity(dir, { 1, 0, 0 });
	if (xTest > bestSimilarity) {
		bestSimilarity = xTest;
		bestResult = { 1, 0, 0 };
	}
	if (-xTest > bestSimilarity) {
		bestSimilarity = -xTest;
		bestResult = { -1, 0, 0 };
	}
	float yTest = cosineSimilarity(dir, { 0, 1, 0 });
	if (yTest > bestSimilarity) {
		bestSimilarity = yTest;
		bestResult = { 0, 1, 0 };
	}
	if (-yTest > bestSimilarity) {
		bestSimilarity = -yTest;
		bestResult = { 0, -1, 0 };
	}
	float zTest = cosineSimilarity(dir, { 0, 1, 0 });
	if (zTest > bestSimilarity) {
		bestSimilarity = zTest;
		bestResult = { 0, 0, 1 };
	}
	if (-zTest > bestSimilarity) {
		bestSimilarity = -zTest;
		bestResult = { 0, 0, -1 };
	}

	// test edge-normals
	float xyTest1 = cosineSimilarity(dir, { 1, 1, 0 });
	if (xyTest1 > bestSimilarity) {
		bestSimilarity = xyTest1;
		bestResult = { 1, 1, 0 };
	}
	if (-xyTest1 > bestSimilarity) {
		bestSimilarity = -xyTest1;
		bestResult = { -1, -1, 0 };
	}
	float xyTest2 = cosineSimilarity(dir, { 1, -1, 0 });
	if (xyTest2 > bestSimilarity) {
		bestSimilarity = xyTest2;
		bestResult = { 1, -1, 0 };
	}
	if (-xyTest2 > bestSimilarity) {
		bestSimilarity = -xyTest2;
		bestResult = { -1, 1, 0 };
	}
	float xzTest1 = cosineSimilarity(dir, { 1, 0, 1 });
	if (xzTest1 > bestSimilarity) {
		bestSimilarity = xzTest1;
		bestResult = { 1, 0, 1 };
	}
	if (-xzTest1 > bestSimilarity) {
		bestSimilarity = -xzTest1;
		bestResult = { -1, 0, -1 };
	}
	float xzTest2 = cosineSimilarity(dir, { 1, 0, -1 });
	if (xzTest2 > bestSimilarity) {
		bestSimilarity = xzTest2;
		bestResult = { 1, 0, -1 };
	}
	if (-xzTest2 > bestSimilarity) {
		bestSimilarity = -xzTest2;
		bestResult = { -1, 0, 1 };
	}
	float yzTest1 = cosineSimilarity(dir, { 0, 1, 1 });
	if (yzTest1 > bestSimilarity) {
		bestSimilarity = yzTest1;
		bestResult = { 0, 1, 1 };
	}
	if (-yzTest1 > bestSimilarity) {
		bestSimilarity = -yzTest1;
		bestResult = { 0, -1, -1 };
	}
	float yzTest2 = cosineSimilarity(dir, { 0, 1, -1 });
	if (yzTest2 > bestSimilarity) {
		bestSimilarity = yzTest2;
		bestResult = { 0, 1, -1 };
	}
	if (-yzTest2 > bestSimilarity) {
		bestSimilarity = -yzTest2;
		bestResult = { 0, -1, 1 };
	}

	// test point-normals
	float xyzTest1 = cosineSimilarity(dir, { 1, 1, 1 });
	if (xyTest1 > bestSimilarity) {
		bestSimilarity = xyTest1;
		bestResult = { 1, 1, 1 };
	}
	if (-xyzTest1 > bestSimilarity) {
		bestSimilarity = -xyTest1;
		bestResult = { -1, -1, -1 };
	}
	float xyzTest2 = cosineSimilarity(dir, { 1, -1, 1 });
	if (xyzTest2 > bestSimilarity) {
		bestSimilarity = xyzTest2;
		bestResult = { 1, -1, 1 };
	}
	if (-xyzTest2 > bestSimilarity) {
		bestSimilarity = -xyzTest2;
		bestResult = { -1, 1, -1 };
	}
	float xyzTest3 = cosineSimilarity(dir, { 1, 1, -1 });
	if (xyzTest3 > bestSimilarity) {
		bestSimilarity = xyzTest3;
		bestResult = { 1, 1, -1 };
	}
	if (-xyzTest3 > bestSimilarity) {
		bestSimilarity = -xyzTest3;
		bestResult = { -1, -1, 1 };
	}
	float xyzTest4 = cosineSimilarity(dir, { 1, -1, -1 });
	if (xyzTest4 > bestSimilarity) {
		bestSimilarity = xyzTest4;
		bestResult = { 1, -1, -1 };
	}
	if (-xyzTest4 > bestSimilarity) {
		bestSimilarity = -xyzTest4;
		bestResult = { -1, 1, 1 };
	}
	
	assert(bestResult != glm::zero<glm::vec3>());
	return bestResult;
}