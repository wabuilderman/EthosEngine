#pragma once
#include "Actor.hpp"
#include "FBXloader.hpp"
#include "DefaultMesh.hpp"
#include "UpdateEvent.hpp"
#include "Message.hpp"

//#include "AnimPose.hpp"
//#include "TestObject2.hpp"

class BasicPistol : public Actor {
public:
	Model* model = nullptr;
	Material myMat = Material();

	Model* boneModel = nullptr;
	RenderComponent* boneRenderComponent = nullptr;

	FBXloader loader;
	AnimPose anim;

	void update(UpdateEvent& e) {
		//static float time = 0;
		//time += e.dt;
	}

	BasicPistol() {
		// Export settings: Geometry - Up=Y, Forward=-Z || Armature - Primary=-Z, Secondary=-X
		loader.loadFBX("models/PistolExport.fbx");

		Mesh* mesh = DefaultMesh::get();


		if (loader.meshes.size() > 0) {
			mesh = &loader.meshes[0];
		}


		model = new Model(mesh, &myMat, nullptr);
		//model = new Model(mesh, &myMat, nullptr);
		addRenderComponent(model);

		//for (size_t i = 0; i < loader.armatures[0].nodeTree.nodes.size(); ++i) {
		//	children.push_back(new TestObject2(this));
		//	children.back()->transformComponent->accessLocalTransform() = model->armature->getBoneModelTransform(i).getMatrix();
		//}

		//children.push_back(new TestObject2(this));

		//physicsComponent = new PhysicsComponent(*this);
		transformComponent = new TransformComponent(*this);
		transformComponent->accessLocalTransform().setScale(glm::vec3(0.1f, 0.1f, 0.1f));

		//transformComponent->accessLocalTransform().setRotation(glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0)));

		Message<UpdateEvent>::Connect(&BasicPistol::update, this);
	}

	~BasicPistol() {
		delete model;
		delete transformComponent;
		//delete physicsComponent;
	}

};