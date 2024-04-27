#pragma once
#include "Actor.hpp"
#include "Message.hpp"
#include "AssetManager.hpp"
#include "UpdateEvent.hpp"
#include "AxisMesh.hpp"

class TestObject2 : public Actor {
public:

	Model* myModel = nullptr;


	void update(UpdateEvent& e) {
	}

	TestObject2(Actor* parent = nullptr) : Actor(parent) {
		myModel = new Model(AxisMesh::getMicro(), nullptr, nullptr);
		addRenderComponent(myModel);
		addTransformComponent();

		//Message<UpdateEvent>::Connect(&TestObject::update2, this);
	}

	~TestObject2() {
		delete myModel;
	}

};