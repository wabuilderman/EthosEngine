#pragma once
#include "Actor.hpp"
#include "Message.hpp"
#include "AssetManager.hpp"
#include "UpdateEvent.hpp"
#include "RenderComponentManager.hpp"

#include "DefaultMesh.hpp"

class MyLight : public Actor {
public:

	//Mesh* myMesh;
	Model* myModel;

	Material myMat;

	RenderComponentManager::LightID lightID;
	float time = 0.0f;
	float speed = 1.0f;
	glm::vec3 color = { 1, 1, 1 };

	void update(UpdateEvent& e) {
		glm::vec3 basePos =
			transformComponent->getLocalTransform().getPosition()
			- glm::vec3({ sinf(time), cosf(time), 0 });

		time += e.dt * speed;

		if(time > glm::two_pi<float>()) {
			time = fmod(time, glm::two_pi<float>());
		}

		transformComponent->accessLocalTransform().setPosition(
			glm::vec3({ sinf(time), cosf(time), 0 }) + basePos
		);

		Light* light = RenderComponentManager::getLight(lightID);
		light->pos = transformComponent->getGlobalTransform().getPosition();
		light->color = glm::vec4(color, 1.0f);
	}

	MyLight() {
		myModel = new Model(DefaultMesh::get(), &myMat, nullptr);
		myMat.emission = { 1.0f, 1.0f, 1.0f, 1.0f };

		addRenderComponent(myModel);
		addTransformComponent();

		transformComponent->accessLocalTransform().scale(0.1f);

		Light lightBlueprint = {
			.pos = { 0, 0, 0 },
			.color = { 1.0f, 1.0f, 0.5f, 1.0f },
			.type = Light::Type::Point
		};

		lightID = RenderComponentManager::addLight(lightBlueprint);

		Message<UpdateEvent>::Connect(&MyLight::update, this);
	}

	~MyLight() {

		RenderComponentManager::removeLight(lightID);

		delete myModel;
	}

};