#pragma once

#include "pch.hpp"

#include "Model.hpp"
#include "BufferManager.hpp"
#include "Texture.hpp"

#include "Component.hpp"
#include "Transform2D.hpp"
#include "WindowWrapper.hpp"
#include <array>

class UIComponent : public Component {
public:
	Model* model = nullptr;
	bool isVisible = true;
	UIComponent* parent = nullptr;

	enum class Alignment { Top = 0, Left = 0, Center = 1, Bottom = 2, Right = 2 };
	enum class ScalingMode { None, Full, UniformVertical, Vertical, UniformHorizontal, Horizontal };

	Alignment vertAlign = Alignment::Center;
	Alignment horzAlign = Alignment::Center;

	ScalingMode scalingMode = ScalingMode::Full;

	float width = 0.0f;
	float height = 0.0f;

	Transform2D transform;
	glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

	UIComponent(Actor& owner, Model* model = nullptr, UIComponent* parent = nullptr);


	void setModel(Model* model);

	glm::mat3 getLocalMatrix() {
		
		float localTop;
		float localBottom;
		float localLeft;
		float localRight;

		if (parent) {
			localTop = -parent->height / 2;
			localBottom = parent->height / 2;
			localLeft = -parent->width / 2;
			localRight = parent->width / 2;
		}
		else {
			localTop = -(float)Window::height / 2;
			localBottom = (float)Window::height / 2;
			localLeft = -(float)Window::width / 2;
			localRight = (float)Window::width / 2;
		}
		
		glm::mat3 result = transform.getMatrix();
		

		// handle vertical alignment
		if (vertAlign == Alignment::Center) {
			result[2][1] -= (height / 2) * transform.getScale().y;
		}

		// handle horizontal alignment
		if (horzAlign == Alignment::Center) {
			result[2][0] -= (width / 2) * transform.getScale().x;;
		}

		return result;
	}

	glm::mat3 getGlobalMatrix() {
		if (parent) {
			if (scalingMode == ScalingMode::None) {
				glm::mat3 mat = parent->getGlobalMatrix() * getLocalMatrix();
				mat[0][0] = mat[0][0] / parent->transform.getScale().x;
				mat[1][1] = mat[1][1] / parent->transform.getScale().y;
				return mat;
			}
			else if (scalingMode == ScalingMode::UniformVertical) {
				glm::mat3 mat = parent->getGlobalMatrix() * getLocalMatrix();
				float ratio = parent->transform.getScale().y / parent->transform.getScale().x;
				mat[0][0] *= ratio;
				mat[1][0] *= ratio;
				mat[2][0] *= ratio;
				return mat;
			}
			else {
				return parent->getGlobalMatrix() * getLocalMatrix();
			}
			
		}
		else {
			return getLocalMatrix();
		}
	}

	glm::mat4 getFullGlobalMatrix() {
		glm::mat3 mat = getGlobalMatrix();
		// Rotate to match object space
		return {
			{ 0,          0,         1, 0 },
			{ mat[1][0],  mat[1][1], 0, 0 },
			{ mat[0][0],  mat[0][1], 0, 0 },
			{ mat[2][0],  mat[2][1], 0, 1 }
		};
	}

	~UIComponent();
};
