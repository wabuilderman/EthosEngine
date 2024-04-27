#pragma once
#include "Actor.hpp"
#include "GenerativeCubicBezierMesh.hpp"
#include "InputManager.hpp"
#include "CameraManager.hpp"
#include "RingCircleMesh.hpp"
#include "UpdateEvent.hpp"
#include "SelectableActor.hpp"

class CurveNodeHandle : public SelectableActor {

public:
	Model* model = nullptr;
	Material myMat = Material();
	Mesh* mesh;

	GenerativeCubicBezierMesh* linkedCurve;
	size_t pointIndex;

	bool isChildOfRoadBase = false;

	bool ignoreExtendButton = false;

	bool doFreeformDrag = false;

	float nodeHeight = 0.0f;

	float time = 0.0f;
	float speed = 1.0f;

	void OnDrag(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) override;

	void OnSelect() override {
		myMat.emission = glm::vec4(1.f, 1.f, 1.f, 1.f);
	}

	void OnDeselect() override;

	void OnDragEnd() override {
		doFreeformDrag = false;
		ignoreExtendButton = false;
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

	void onKeyPress(KeyPress& e);
	
	bool isEndOfRoad() const;

	CurveNodeHandle(GenerativeCubicBezierMesh* linkedCurve, size_t pointIndex) : linkedCurve(linkedCurve), pointIndex(pointIndex) {

		colliderMeasure1 = 0.1f;

		mesh = new RingCircleMesh(1.f, 0.9f, 2, glm::vec3(0.f, 0.2f, 1.f), glm::vec3(1.f, 1.f, 1.f));
		myMat.emission = glm::vec4(0.2f, 0.2f, 0.2f, 0.2f);
		model = new Model(mesh, &myMat, nullptr);
		addRenderComponent(model);
		transformComponent = new TransformComponent(*this);
		transformComponent->accessLocalTransform().setScale(glm::vec3(0.1f, 0.1f, 0.1f));
		transformComponent->accessLocalTransform().setRotation(glm::quat(0, 0.707f, 0.0f, -0.707f));

		if (linkedCurve != nullptr) {
			transformComponent->accessLocalTransform().setPosition(
				linkedCurve->curve.points[pointIndex] + Vector::up * 0.3f);
		}
		Message<KeyPress>::Connect(&CurveNodeHandle::onKeyPress, this);
	}

	void hide() {
		RenderComponentManager::updateRenderComponentVisibility(renderComponents[0], false);
		colliderMode = SelectionColliderMode::disabled;
	}

	void reveal() {
		RenderComponentManager::updateRenderComponentVisibility(renderComponents[0], true);
		colliderMode = SelectionColliderMode::sphere;
	}

	~CurveNodeHandle() {

		removeRenderComponents();

		delete model;
		delete mesh;

		Message<KeyPress>::Disconnect(&CurveNodeHandle::onKeyPress, this);
	}

};