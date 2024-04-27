#pragma once
#include "Actor.hpp"
#include "AxisMesh.hpp"

class WorldSpaceMarker : public Actor {
	Mesh* mesh;
	Model* model;

public:

	WorldSpaceMarker() {
		mesh = new AxisMesh();
		model = new Model(mesh, nullptr, nullptr);

		addRenderComponent(model);
		transformComponent = new TransformComponent(*this);

		Transform& transform = transformComponent->accessLocalTransform();
		transform.scale(glm::vec3(0.1f, 0.1f, 0.1f));

	}

	~WorldSpaceMarker() {
		removeRenderComponents();
		delete mesh;
		delete model;
	}

};