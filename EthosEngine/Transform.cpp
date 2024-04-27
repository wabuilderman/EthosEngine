#include "pch.hpp"
#include "Transform.hpp"

void Transform::setPosition(glm::vec3 position) {
	T = position;
	isDirty = true;
}
void Transform::setRotation(glm::quat rotation) {
	R = rotation;
	isDirty = true;
}
void Transform::setScale(glm::vec3 scale) {
	S = scale;
	isDirty = true;
}

void Transform::moveRelative(glm::vec3 delta) {
	T += glm::vec3(glm::rotate(R,glm::vec4(delta, 0.0f)));
		
		//glm::cross(R, delta*glm::vec3(1.0f, 1.0f, 1.0f));
	
	isDirty = true;
}
void Transform::moveAbsolute(glm::vec3 delta) {
	T += delta;
	isDirty = true;
}

void Transform::rotateRelative(glm::quat delta) {
	R *= delta;
	isDirty = true;
}
void Transform::rotateAbsolute(glm::quat delta) {
	R = delta * R;
	isDirty = true;
}
void Transform::addVectorRotation(glm::vec3 v, float t) {
	R = glm::normalize(R + 0.5f * t * glm::quat(0.0f, v.x, v.y, v.z) * R);
	isDirty = true;
}

void Transform::scale(glm::vec3 delta) {
	S *= delta;
	isDirty = true;
}

void Transform::scale(float delta) {
	S *= delta;
	isDirty = true;
}


glm::vec3 Transform::getPosition() const {
	return T;
}
glm::quat Transform::getRotation() const {
	return R;
}
glm::vec3 Transform::getScale() const {
	return S;
}

void Transform::clean() {
	if (isDirty) {
		transform = glm::translate(glm::mat4(1.0f), T) * glm::mat4_cast(R) * glm::scale(glm::mat4(1.0f), S);
		isDirty = false;
	}
}

glm::mat4 Transform::getMatrix() const {
	// Secretly modify anyways; lazy calculation
	const_cast<Transform*>(this)->clean();
	return transform;
}

Transform Transform::lerp(const Transform& a, const Transform& b, float t) {
	glm::vec3 translation = a.T * (1 - t) + b.T * t;
	glm::quat rotation = glm::slerp(a.R, b.R, t);
	glm::vec3 scale = a.S * (1 - t) + b.S * t;
	return Transform(translation, rotation, scale);
}