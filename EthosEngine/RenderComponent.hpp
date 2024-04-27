#pragma once
#include "pch.hpp"

#include "Model.hpp"
#include "BufferManager.hpp"
#include "Texture.hpp"

#include "Component.hpp"

#include <array>

class RenderComponent : public Component {
public:
	Model* model = nullptr;
	bool isVisible = true; // WARNING: Use render component manager to change visibility

	RenderComponent(Actor& owner, Model* model = nullptr);

	void setModel(Model * model);

	~RenderComponent();
};
