#include "Transform2D.hpp"

void Transform2D::setPosition(glm::vec2 position) {
	T = position;
	isDirty = true;
}
void Transform2D::setRotation(float rotation) {
	R = rotation;
	isDirty = true;
}
void Transform2D::setScale(glm::vec2 scale) {
	S = scale;
	isDirty = true;
}

void Transform2D::moveRelative(glm::vec2 delta) {
	float c = cosf(R); float s = sinf(R);
	T += glm::vec2({c*delta.x - s*delta.y, s*delta.x + c*delta.y });

	isDirty = true;
}
void Transform2D::moveAbsolute(glm::vec2 delta) {
	T += delta;
	isDirty = true;
}

void Transform2D::rotate(float rotation) {
	R += rotation;
	isDirty = true;
}
void Transform2D::scale(glm::vec2 delta) {
	S *= delta;
	isDirty = true;
}
void Transform2D::scale(float delta) {
	S *= delta;
	isDirty = true;
}


glm::vec2 Transform2D::getPosition() const {
	return T;
}
float Transform2D::getRotation() const {
	return R;
}
glm::vec2 Transform2D::getScale() const {
	return S;
}

void Transform2D::clean() {
	float c = cosf(R); float s = sinf(R);
	if (isDirty) {
		transform =
			glm::mat3({ 1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,   T.x, T.y, 1.0f }) *  // T
			glm::mat3({ c,    -s,    0.0f,  s,    c,    0.0f,   0.0f, 0.0f, 1.0f }) *  // R
			glm::mat3({ S.x,  0.0f, 0.0f,   0.0f, S.y,  0.0f,   0.0f, 0.0f, 1.0f });   // S
			
		isDirty = false;
	}
}

glm::mat3 Transform2D::getMatrix() const {
	// Secretly modify anyways; lazy calculation
	const_cast<Transform2D*>(this)->clean();
	return transform;
}