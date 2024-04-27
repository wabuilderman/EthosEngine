#pragma once
#include "Actor.hpp"
#include "FBXloader.hpp"
#include "DefaultMesh.hpp"
#include "UpdateEvent.hpp"
#include "Message.hpp"

#include "AnimPose.hpp"
#include "TestObject2.hpp"

class TestObject : public Actor {
public:
	Model* model = nullptr;
	Material myMat = Material();

	Model* boneModel = nullptr;
	RenderComponent* boneRenderComponent = nullptr;

	FBXloader loader;
	AnimPose anim;

	void update(UpdateEvent& e) {
		static float time = 0;
		time += e.dt;

		//constexpr float interval = glm::two_pi<float>();
		float interval = (float)loader.animations[1].length;

		if (time > interval) {
			time -= interval;
		}
		const Pose& pose = loader.animations[1].getPose(time);
		//loader.armatures[0].applyPose(pose);

		//for (size_t i = 0; i < loader.armatures[0].nodeTree.nodes.size(); ++i) {
		//	children[i]->transformComponent->accessLocalTransform() = loader.armatures[0].getBoneModelTransform(i).getMatrix();
		//}
	}

	TestObject() {
		// Export settings: Geometry - Up=Y, Forward=-Z || Armature - Primary=-Z, Secondary=-X
		loader.loadFBX("models/fox.fbx");

		Mesh* mesh = DefaultMesh::get();


		if (loader.meshes.size() > 0) {
			mesh = &loader.meshes[0];
		}


		model = new Model(mesh, &myMat, &loader.armatures[0]);
		//model = new Model(mesh, &myMat, nullptr);
		addRenderComponent(model);

		model->armature->applyPose(loader.animations[0].getPose(0));//loader.animations[0].length));
		model->armature->setRefPose();

		//for (size_t i = 0; i < loader.armatures[0].nodeTree.nodes.size(); ++i) {
		//	children.push_back(new TestObject2(this));
		//	children.back()->transformComponent->accessLocalTransform() = model->armature->getBoneModelTransform(i).getMatrix();
		//}

		//children.push_back(new TestObject2(this));

		addTransformComponent();

		transformComponent->accessLocalTransform().setScale(glm::vec3(3, 3, 3));

		//transformComponent->accessLocalTransform().setRotation(glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0)));

		Message<UpdateEvent>::Connect(&TestObject::update, this);
	}

	~TestObject() {
		delete model;
	}

};