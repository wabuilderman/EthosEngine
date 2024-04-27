#pragma once
#include "pch.hpp"

class Vector2D : public glm::vec2 {
public:
	Vector2D(glm::vec2& vec) : glm::vec2(vec) {}
	Vector2D(const glm::vec2& vec) : glm::vec2(vec) {}

	static constexpr glm::vec2 up = { 0.0f, 1.0f };
	static constexpr glm::vec2 down = { 0.0f, -1.0f };
	static constexpr glm::vec2 left = { -1.0f, 0.0f };
	static constexpr glm::vec2 right = { 1.0f, 0.0f };

	static constexpr glm::vec2 center = { 0.0f, 0.0f };

};

class Transform2D {

	glm::mat3 transform;
	bool isDirty = true;

	glm::vec2 T = { 0, 0 };
	float R;
	glm::vec2 S = { 1, 1 };

public:
	Transform2D() { clean(); }

	void setPosition(glm::vec2 position);
	glm::vec2 getPosition() const;
	void moveAbsolute(glm::vec2 delta);
	void moveRelative(glm::vec2 delta);

	void setRotation(float rotation);
	float getRotation() const;
	void rotate(float rotation);

	void setScale(glm::vec2 scale);
	glm::vec2 getScale() const;
	void scale(glm::vec2 delta);
	void scale(float delta);

	void clean();
	glm::mat3 getMatrix() const;
};