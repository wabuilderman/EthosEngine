#pragma once
#include "pch.hpp"

#include "Model.hpp"
#include "BufferManager.hpp"
#include "Texture.hpp"

#include "Component.hpp"

#include <array>


class DebugVisualizerComponent : public Component {
public:
	enum class VisualizationType {
		line,
		arrow,
		axis
	};

	bool isVisible = true;
	Model* model = nullptr;

	glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	Transform globalTransform;

	VisualizationType getVisualizationType() const;
	void setVisualizationType(VisualizationType type);

	DebugVisualizerComponent(Actor& owner);
	~DebugVisualizerComponent();

private:
	VisualizationType visualizationType = VisualizationType::axis;
};