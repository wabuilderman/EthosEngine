#pragma once
#include "BasicPistol.hpp"
#include "MyText.hpp"
#include "TestObject.hpp"
#include "TestObject2.hpp"
#include "testPhysObj.hpp"
#include "testSphere.hpp"
#include "WorldSpaceMarker.hpp"
#include "MyPlayer.hpp"
#include "MyFloor.hpp"
#include "MyLight.hpp"
#include "ObjectPicker.hpp"
#include "RoadBuilder.hpp"
#include "PhysicsManager.hpp"
#include "IntersectionBase.hpp"
#include "Scene.hpp"
#include "VehiclePath.hpp"

class MyTestScene : public Scene {
public:
	//TestPhysObj* test1 = nullptr;
	//TestPhysObj* test2 = nullptr;

	static float randRange(float min, float max) {
		return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (max - min) + min;
	}

	bool firstUpdate = true;

	void spawnLight() {
		MyLight* light = new MyLight();
		actors.push_back(light);

		light->speed = randRange(0.7f, 2.0f);
		light->time = randRange(0.0f, 3.0f);

		light->color = glm::normalize(glm::vec3({
			randRange(0.2f, 1.0f),
			randRange(0.2f, 1.0f),
			randRange(0.2f, 1.0f)
		}));

		light->myModel->material->emission = {
			light->color.x,
			light->color.y,
			light->color.z,
			1.0f
		};

		light->transformComponent->accessLocalTransform().setPosition({
			randRange(2.0f, 4.0f),
			randRange(2.0f, 4.0f),
			randRange(1.2f, 2.4f)
		});
	}

	void update(UpdateEvent& e) {
	}

	MyTestScene() {
		actors.push_back(new MyPlayer());
		actors.back()->teleport(Vector::backward * 1.5f);
		actors.push_back(new WorldSpaceMarker());

		actors.push_back(new WorldSpaceMarker());
		actors.back()->teleport(Vector::forward * 1.0f);

		actors.push_back(new WorldSpaceMarker());
		actors.back()->teleport(Vector::forward * 0.5f);


		actors.push_back(new MyFloor());
		actors.back()->teleport(Vector::forward * 4.0f + Vector::right * 0.0f + Vector::down * 0.5f);
		

		Message<UpdateEvent>::Connect(&MyTestScene::update, this);

		RoadBuilder* roadBuilder = new RoadBuilder();
		actors.push_back(roadBuilder);

		IntersectionBase* intersection1 = new IntersectionBase();
		roadBuilder->addIntersection(intersection1);
		intersection1->teleport({ 2.5f, 2.5f, 0.f });

		IntersectionBase* intersection2 = new IntersectionBase();
		roadBuilder->addIntersection(intersection2);
		intersection2->teleport({ 2.5f, 6.f, 0.f });

		roadBuilder->addRoad(new RoadBase(
			glm::vec3(0.0f, 2.5f, 0),
			glm::vec3(1.5f, 2.5f, 0)));
		intersection1->ConnectRoad(roadBuilder->roads.back(), true);

		roadBuilder->addRoad(new RoadBase(
			glm::vec3(2.5f, 0.0f, 0),
			glm::vec3(2.5f, 1.5f, 0)));
		intersection1->ConnectRoad(roadBuilder->roads.back(), true);

		roadBuilder->addRoad(new RoadBase(
			glm::vec3(3.5f, 2.5f, 0),
			glm::vec3(5.0f, 2.5f, 0)));
		intersection1->ConnectRoad(roadBuilder->roads.back(), false);

		roadBuilder->addRoad(new RoadBase(
			glm::vec3(2.5f, 3.5f, 0),
			glm::vec3(2.5f, 5.0f, 0)));
		intersection1->ConnectRoad(roadBuilder->roads.back(), false);
		intersection2->ConnectRoad(roadBuilder->roads.back(), true);

		roadBuilder->addRoad(new RoadBase(
			glm::vec3(3.5f, 6.f, 0),
			glm::vec3(5.0f, 6.f, 0)));
		intersection2->ConnectRoad(roadBuilder->roads.back(), false);

		roadBuilder->addRoad(new RoadBase(
			glm::vec3(2.5f, 7.f, 0),
			glm::vec3(2.5f, 8.5f, 0)));
		intersection2->ConnectRoad(roadBuilder->roads.back(), false);

		roadBuilder->addRoad(new RoadBase(
			glm::vec3(0.0f, 6.f, 0),
			glm::vec3(1.5f, 6.f, 0)));
		intersection2->ConnectRoad(roadBuilder->roads.back(), true);

		VehiclePath* vehiclePath = new VehiclePath(roadBuilder->actorID);
		actors.push_back(vehiclePath);



		spawnLight();
	}

	~MyTestScene() {
	}
};