#pragma once
#include "pch.hpp"

class Vector : public glm::vec3 {
public:
	Vector(float x, float y, float z) : glm::vec3(x, y, z) {}
	Vector(glm::vec3& vec) : glm::vec3(vec) {}
	Vector(const glm::vec3& vec) : glm::vec3(vec) {}

	static constexpr glm::vec3 up = { 0.0f, 0.0f, 1.0f };
	static constexpr glm::vec3 right = { 0.0f, 1.0f, 0.0f };
	static constexpr glm::vec3 forward = { 1.0f, 0.0f, 0.0f };

	static constexpr glm::vec3 down = { 0.0f, 0.0f, -1.0f };
	static constexpr glm::vec3 left = { 0.0f, -1.0f, 0.0f };
	static constexpr glm::vec3 backward = { -1.0f, 0.0f, 0.0f };

	static constexpr glm::vec3 zero = { 0.0f, 0.0f, 0.0f };

	static inline glm::vec3 damp(glm::vec3 v, float t) {
		v.x -= copysignf(fmin(fabsf(v.x), t), v.x);
		v.y -= copysignf(fmin(fabsf(v.y), t), v.y);
		v.z -= copysignf(fmin(fabsf(v.z), t), v.z);
		return v;
	}
};

class Quaternion : public glm::quat {
public:
	Quaternion(float x, float y, float z, float w) : glm::quat(x, y, z, w) {}

	Quaternion(glm::quat& quat) : glm::quat(quat) {}
	Quaternion(const glm::quat& quat) : glm::quat(quat) {}
};

class Transform {
public:
	glm::mat4 transform;
	bool isDirty = true;

	glm::vec3 T;
	glm::quat R;
	glm::vec3 S;

	Transform(glm::vec3 T, glm::quat R, glm::vec3 S) : T(T), R(R), S(S) { clean(); }
	Transform(const glm::mat4& ref = glm::mat4(1)) : transform(ref) {
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(ref, S, R, T, skew, perspective);
		isDirty = false;
	}

	void setPosition(glm::vec3 position);
	glm::vec3 getPosition() const;
	void moveAbsolute(glm::vec3 delta);
	void moveRelative(glm::vec3 delta);

	void setRotation(glm::quat rotation);
	glm::quat getRotation() const;
	void rotateAbsolute(glm::quat delta);
	void rotateRelative(glm::quat delta);
	void addVectorRotation(glm::vec3 v, float t);

	void setScale(glm::vec3 scale);
	glm::vec3 getScale() const;
	void scale(glm::vec3 delta);
	void scale(float delta);

	void clean();
	glm::mat4 getMatrix() const;

	static Transform lerp(const Transform& a, const Transform& b, float t);
};