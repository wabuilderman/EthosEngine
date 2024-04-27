#pragma once
#include "Actor.hpp"
#include "FBXloader.hpp"
#include "DefaultMesh.hpp"
#include "UpdateEvent.hpp"
#include "Message.hpp"

#include "AnimPose.hpp"

class TestSphere : public Actor {
public:
	Model* model = nullptr;
	FBXloader loader;
	void update(UpdateEvent& e) {
		
	}

	TestSphere() {
		loader.loadFBX("models/sphere.fbx");
		Mesh* mesh = DefaultMesh::get();
		if (loader.meshes.size() > 0) {
			mesh = &loader.meshes[0];
		}

		model = new Model(mesh, nullptr, nullptr);
		addRenderComponent(model);
		transformComponent = new TransformComponent(*this);
	}

	~TestSphere() {
		delete model;
	}

};