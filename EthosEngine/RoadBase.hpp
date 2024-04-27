#pragma once
#include "Actor.hpp"
#include "MyControls.hpp"
#include "FBXloader.hpp"
#include "DefaultMesh.hpp"
#include "UpdateEvent.hpp"
#include "Message.hpp"
#include "Mesh.hpp"
#include "Transform.hpp"
#include "RingCircleMesh.hpp"
#include "RenderComponentManager.hpp"
#include "GenerativeCubicBezierMesh.hpp"
#include <sstream>
#include "CurveNodeHandle.hpp"
#include "GraphicsPipeline.hpp"


class RoadBase : public SelectableActor {
public:
	Texture* textureC = nullptr;
	Texture* textureLineColor = nullptr;
	Texture* textureH = nullptr;
	Texture* textureN = nullptr;
	Texture* textureR = nullptr;
	Texture* textureAO = nullptr;
	Model* roadModel = nullptr;
	Model* lineModel = nullptr;
	Material myMat = Material();
	Material lineMat = Material();
	Mesh* roadMesh = nullptr;
	Mesh* lineMesh = nullptr;

	float time = 0.0f;
	bool showCurveNodes = false;

	std::vector<size_t> curveNodeChildIndices;

	ActorID startIntersection = (size_t) -1;
	ActorID endIntersection = (size_t) -1;

	// TODO: Implement actual length calculations
	float getLength() {
		return 1.f;
	}

	size_t getNumSegements() const {
		return curveNodeChildIndices.size() / 3;
	}

	CurveNodeHandle* getCurveNode(size_t index) {
		return (CurveNodeHandle*)children[curveNodeChildIndices[index]];
	}

	ActorID splitAtTPos(float tPos) {
		RoadBase* newestRoad = new RoadBase(((GenerativeCubicBezierMesh*)this->roadMesh)->curve.getSecondOfSplit(tPos));
		this->parent->addChild(newestRoad);
		newestRoad->endIntersection = endIntersection;
		newestRoad->startIntersection = startIntersection;
		
		ComplexCubicBezierCurve curve = ((GenerativeCubicBezierMesh*)this->roadMesh)->curve.getFirstOfSplit(tPos);
		size_t numPoints = curve.points.size();

		// trim excess points
		for (size_t i = numPoints; i < curveNodeChildIndices.size(); ++i) {
			delete children[curveNodeChildIndices[i]];
			children.erase(children.begin() + i);
		}
		curveNodeChildIndices.resize(numPoints);
		for (size_t index : curveNodeChildIndices) {
			children[index]->teleport(curve.points[index]);
		}

		UpdatePoint(3, ((Actor*)children[curveNodeChildIndices[3]])->transformComponent->getLocalTransform().getPosition() - Vector::up * 0.3f);
		return newestRoad->actorID;
	}


	void configureMaterials() {
		textureC = new Texture("textures/pebbled-asphalt1/pebbled_asphalt_albedo_2048.png");
		myMat.setTexture(textureC, Material::TextureType::Color);

		textureR = new Texture("textures/pebbled-asphalt1/pebbled_asphalt_Roughness_512.png");
		myMat.setTexture(textureR, Material::TextureType::Specular);
		myMat.roughness = 1.f;
		myMat.baseColor = glm::vec3(0.5f, 0.5f, 0.5f);


		textureAO = new Texture("textures/pebbled-asphalt1/pebbled_asphalt_ao_512.png");
		myMat.setTexture(textureAO, Material::TextureType::Ambient);

		myMat.diffuseUVScale = glm::vec2(2.f, 2.f);
		myMat.ambientUVScale = glm::vec2(2.f, 2.f);
		myMat.specularUVScale = glm::vec2(2.f, 2.f);

		textureLineColor = new Texture("textures/dashedLine.png");

		lineMat.setTexture(textureLineColor, Material::TextureType::Color);
		lineMat.setTexture(textureR, Material::TextureType::Specular);
		lineMat.setTexture(textureAO, Material::TextureType::Ambient);
		lineMat.diffuseUVScale = glm::vec2(5.f, 1.f);
		lineMat.roughness = 1.f;
		lineMat.sheen = 0.f;
	}

	void ExtendRoad(bool atStart) {
		size_t numSegments = getNumSegements();
		glm::vec3 newPos = children[curveNodeChildIndices.back()]->transformComponent->getGlobalTransform().getPosition();
		((GenerativeCubicBezierMesh*)lineMesh)->curve.extendCurve(newPos, newPos, newPos, atStart);
		((GenerativeCubicBezierMesh*)roadMesh)->curve.extendCurve(newPos, newPos, newPos, atStart);

		CurveNodeHandle* control1;
		CurveNodeHandle* control2;
		CurveNodeHandle* endPoint;

		if (atStart) {
			curveNodeChildIndices.resize(curveNodeChildIndices.size() + 3);

			for (size_t i = curveNodeChildIndices.size() - 3; i > 0; --i) {
				curveNodeChildIndices[i + 2] = curveNodeChildIndices[i - 1];
				curveNodeChildIndices[i - 1] = -1;
				((CurveNodeHandle*)children[curveNodeChildIndices[i + 2]])->pointIndex += 3;
			}

			control1 = new CurveNodeHandle((GenerativeCubicBezierMesh*)roadMesh, 2);
			addChild(control1);
			control1->isChildOfRoadBase = true;
			curveNodeChildIndices[2] = children.size() - 1;

			control2 = new CurveNodeHandle((GenerativeCubicBezierMesh*)roadMesh, 1);
			addChild(control2);
			control2->isChildOfRoadBase = true;
			curveNodeChildIndices[1] = children.size() - 1;

			endPoint = new CurveNodeHandle((GenerativeCubicBezierMesh*)roadMesh, 0);
			addChild(endPoint);
			endPoint->isChildOfRoadBase = true;
			curveNodeChildIndices[0] = children.size() - 1;
		}
		else {
			control1 = new CurveNodeHandle((GenerativeCubicBezierMesh*)roadMesh, numSegments * 3 + 1);
			addChild(control1);
			control1->isChildOfRoadBase = true;
			curveNodeChildIndices.push_back(children.size() - 1);
			control2 = new CurveNodeHandle((GenerativeCubicBezierMesh*)roadMesh, numSegments * 3 + 2);
			addChild(control2);
			control2->isChildOfRoadBase = true;
			curveNodeChildIndices.push_back(children.size() - 1);

			endPoint = new CurveNodeHandle((GenerativeCubicBezierMesh*)roadMesh, numSegments * 3 + 3);
			addChild(endPoint);
			endPoint->isChildOfRoadBase = true;
			curveNodeChildIndices.push_back(children.size() - 1);
		}
		endPoint->doFreeformDrag = true;
		endPoint->Select();
		endPoint->SetDragState(true);
		MyControls::getControls().objectSelector.isDragging = true;
		endPoint->ignoreExtendButton = true;
	}

	void revealCurveNodes() {
		showCurveNodes = true;
		for (size_t index : curveNodeChildIndices) {
			((CurveNodeHandle*)children[index])->reveal();
		}
	}

	void hideCurveNodes() {
		showCurveNodes = false;
		for (size_t index : curveNodeChildIndices) {
			((CurveNodeHandle*)children[index])->hide();
		}
	}

	void OnSelect() override {
		myMat.emission = glm::vec4(1.f, 1.f, 1.f, 1.f);
	}
	void OnDeselect() override {
		myMat.emission = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		if (showCurveNodes) {
			hideCurveNodes();
		}
	}

	void OnDrag(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) override {

	}

	void OnMouseoverBegin() override {
		if (!IsSelected()) {
			myMat.emission = glm::vec4(0.5f, 0.5f, 0.5f, 0.5f);
		}
	}

	void OnMouseoverEnd() override {
		if (!IsSelected()) {
			myMat.emission = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}

	void onKeyPress(KeyPress& e) {
		if (e.keycode == GLFW_KEY_T && IsSelected()) {
			revealCurveNodes();
		}
	}



	void UpdatePoint(size_t pointIndex, glm::vec3 position);

	RoadBase(ComplexCubicBezierCurve curve) {
		configureMaterials();

		roadMesh = new GenerativeCubicBezierMesh(curve, 500, 1.0f);
		lineMesh = new GenerativeCubicBezierMesh(curve, 500, 0.03f, 0.201, 0.f, 0.f, true, false, false, false, false, false);

		colliderMesh = roadMesh;
		colliderMode = SelectionColliderMode::mesh;

		// create all the nodes needed
		size_t numNodes = curve.points.size();
		for (size_t i = 0; i < numNodes; ++i) {
			addChild(new CurveNodeHandle((GenerativeCubicBezierMesh*)roadMesh, i));
			((CurveNodeHandle*)children.back())->isChildOfRoadBase = true;
			curveNodeChildIndices.push_back(children.size() - 1);
		}

		roadModel = new Model(roadMesh, &myMat, nullptr);
		lineModel = new Model(lineMesh, &lineMat, nullptr);

		UpdatePoint(3, ((Actor*)children[curveNodeChildIndices[3]])->transformComponent->getLocalTransform().getPosition() - Vector::up * 0.3f);
		addRenderComponent(roadModel);
		addRenderComponent(lineModel);
		addTransformComponent();
		hideCurveNodes();
		Message<KeyPress>::Connect(&RoadBase::onKeyPress, this);
	}

	RoadBase(glm::vec3 startingNode, glm::vec3 endingNode, CurveNodeHandle* startingNodeHandle = nullptr) {
		configureMaterials();

		ComplexCubicBezierCurve curve(startingNode, startingNode, endingNode, endingNode);

		roadMesh = new GenerativeCubicBezierMesh(curve, 500, 1.0f);
		lineMesh = new GenerativeCubicBezierMesh(curve, 500, 0.03f, 0.201, 0.f, 0.f, true, false, false, false, false, false);

		colliderMesh = roadMesh;
		colliderMode = SelectionColliderMode::mesh;

		if (startingNodeHandle != nullptr) {
			addChild(startingNodeHandle);
			//startingNodeHandle->transformComponent->accessLocalTransform().moveAbsolute(Vector::up * 0.3f);
			startingNodeHandle->linkedCurve = (GenerativeCubicBezierMesh*)roadMesh;
			startingNodeHandle->Deselect();
		}
		else {
			addChild(new CurveNodeHandle((GenerativeCubicBezierMesh*)roadMesh, 0));
		}
		((CurveNodeHandle*)children.back())->isChildOfRoadBase = true;
		curveNodeChildIndices.push_back(children.size() - 1);
		
		addChild(new CurveNodeHandle((GenerativeCubicBezierMesh*)roadMesh, 1));
		curveNodeChildIndices.push_back(children.size() - 1);
		((CurveNodeHandle*)children.back())->isChildOfRoadBase = true;

		addChild(new CurveNodeHandle((GenerativeCubicBezierMesh*)roadMesh, 2));
		curveNodeChildIndices.push_back(children.size() - 1);
		((CurveNodeHandle*)children.back())->isChildOfRoadBase = true;

		addChild(new CurveNodeHandle((GenerativeCubicBezierMesh*)roadMesh, 3));
		curveNodeChildIndices.push_back(children.size() - 1);
		((CurveNodeHandle*)children.back())->isChildOfRoadBase = true;

		UpdatePoint(3, ((Actor*)children[curveNodeChildIndices[3]])->transformComponent->getLocalTransform().getPosition()-Vector::up*0.3f);
		roadModel = new Model(roadMesh, &myMat, nullptr);

		lineModel = new Model(lineMesh, &lineMat, nullptr);

		addRenderComponent(roadModel);
		addRenderComponent(lineModel);
		addTransformComponent();
		hideCurveNodes();
		Message<KeyPress>::Connect(&RoadBase::onKeyPress, this);
	}

	~RoadBase() {
		removeRenderComponents();
		removeTransformComponent();
		Message<KeyPress>::Disconnect(&RoadBase::onKeyPress, this);

		delete textureC;
		delete textureLineColor;
		//delete textureH;
		//delete textureN;
		delete textureR;
		delete textureAO;

		delete lineModel;
		delete lineMesh;

		delete roadModel;
		delete roadMesh;
	}

private:
	void UpdatePointForCurve(size_t pointIndex, glm::vec3 position, ComplexCubicBezierCurve& curve);
};