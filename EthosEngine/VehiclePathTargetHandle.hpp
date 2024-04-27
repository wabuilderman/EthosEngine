#pragma once
#include "SelectableActor.hpp"
#include "RingCircleMesh.hpp"
#include "UpdateEvent.hpp"


class VehiclePathTargetHandle : public SelectableActor {

public:
	Model* model = nullptr;
	Material myMat = Material();
	Mesh* mesh;

	ActorID vehiclePathID;
	bool isStartHandle = true;

	float nodeHeight = 0.0f;

	float time = 0.0f;
	float speed = 1.0f;

	void OnDrag(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) override;

	void OnSelect() override {
		myMat.emission = glm::vec4(1.f, 1.f, 1.f, 1.f);
	}

	void OnDeselect() override {
		myMat.emission = glm::vec4(0.2f, 0.2f, 0.2f, 0.2f);
	}

	void OnMouseoverBegin() override {
		if (!IsSelected()) {
			myMat.emission = glm::vec4(0.5f, 0.5f, 0.5f, 0.5f);
		}
	}

	void OnMouseoverEnd() override {
		if (!IsSelected()) {
			myMat.emission = glm::vec4(0.2f, 0.2f, 0.2f, 0.2f);
		}
	}


	VehiclePathTargetHandle(ActorID vehiclePathID, bool isStartHandle) : vehiclePathID(vehiclePathID), isStartHandle(isStartHandle) {

		colliderMeasure1 = 0.1f;

		mesh = new RingCircleMesh(1.f, 0.9f, 2, glm::vec3(0.f, 0.2f, 1.f), glm::vec3(1.f, 1.f, 1.f));
		myMat.emission = glm::vec4(0.2f, 0.2f, 0.2f, 0.2f);
		model = new Model(mesh, &myMat, nullptr);
		addRenderComponent(model);
		transformComponent = new TransformComponent(*this);
		transformComponent->accessLocalTransform().setScale(glm::vec3(0.1f, 0.1f, 0.1f));
		transformComponent->accessLocalTransform().setRotation(glm::quat(0, 0.707f, 0.0f, -0.707f));
	}

	~VehiclePathTargetHandle() {

		removeRenderComponents();

		delete model;
		delete mesh;
	}

};