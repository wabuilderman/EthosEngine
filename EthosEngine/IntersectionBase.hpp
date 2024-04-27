#pragma once
#include "Actor.hpp"
#include "Message.hpp"
#include "UpdateEvent.hpp"
#include "GenerativeIntersectionMesh.hpp"
class IntersectionBase : public SelectableActor {
public:
	// Connected roads, alongside a bool which is true if the road is connecting with its start, false if conncting with its end
	std::vector<std::pair<ActorID, bool>> connectedRoads;
	Texture* textureC;
	Material myMat = Material();
	Model* model = nullptr;
	Mesh* mesh;

	bool isMeshDirty = true;

	void OnSelect() override {
		myMat.emission = glm::vec4(1.f, 1.f, 1.f, 1.f);
	}
	void OnDeselect() override {
		myMat.emission = glm::vec4(0.2f, 0.2f, 0.2f, 0.2f);
	}

	void OnDrag(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) override {
		//glm::vec3 mouseWorldPos = ObjectSelector::castMouseOntoPlane(Vector::up * 0.3f, Vector::up);
		//if (!isnan(mouseWorldPos.x)) {
			//teleport(mouseWorldPos + Vector::up * this->transformComponent->getLocalTransform().getPosition().z);
		//}
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

	void update(UpdateEvent& e) {
		if (isMeshDirty) {
			updateMesh();
		}
	}

	void updateMesh() {

		GenerativeIntersectionMesh* genMesh = (GenerativeIntersectionMesh*)mesh;
		genMesh->edgePoints.clear();
		genMesh->edgeVectors.clear();

		for (std::pair<ActorID, bool> roadEntry : connectedRoads) {

			RoadBase* road = (RoadBase*)ActorManager::getActorByID(roadEntry.first);
			GenerativeCubicBezierMesh* roadMesh = (GenerativeCubicBezierMesh*)road->roadMesh;

			float t = roadEntry.second ? 1.f : 0.f;
			glm::vec3 center = roadMesh->curve.interpolate(t) - this->transformComponent->getGlobalTransform().getPosition();
			glm::vec3 derivative = glm::normalize(roadMesh->curve.derivative(t));
			glm::vec3 binorm = glm::cross(derivative, Vector::up);

			if (roadEntry.second) {
				genMesh->edgeVectors.push_back(derivative);
				genMesh->edgePoints.push_back(center - binorm * roadMesh->width * 0.5f);
				genMesh->edgePoints.push_back(center + binorm * roadMesh->width * 0.5f);
			}
			else {
				genMesh->edgeVectors.push_back(-derivative);
				genMesh->edgePoints.push_back(center + binorm * roadMesh->width * 0.5f);
				genMesh->edgePoints.push_back(center - binorm * roadMesh->width * 0.5f);
			}
		}

		((GenerativeIntersectionMesh*)mesh)->calculateMeshData();

		if (isMeshDirty) {
			// Fully augments mesh; handles vertex-count change
			//RenderComponentManager::removeMesh(mesh);
			removeRenderComponents();
			addRenderComponent(model);
		}
		else {
			// Simply moves around vertices in existing mesh
			RenderComponentManager::updateMesh(mesh);
		}
		isMeshDirty = false;
		
	}

	void ConnectRoad(ActorID roadID, bool direction) {

		//TODO remove whatever road was previously connected at that end

		RoadBase* road = (RoadBase*)ActorManager::getActorByID(roadID);
		if (direction) {
			road->startIntersection = this->actorID;
		}
		else {
			road->endIntersection = this->actorID;
		}
		connectedRoads.push_back(std::pair<ActorID, bool>(roadID, direction));
		isMeshDirty = true;
	}

	IntersectionBase() {
		//colliderMeasure1 = 0.5f;

		Message<UpdateEvent>::Connect(&IntersectionBase::update, this);

		textureC = new Texture("textures/testGrid.png");
		myMat.setTexture(textureC, Material::TextureType::Color);

		std::vector<glm::vec3> edgePoints;
		std::vector<glm::vec3> edgeVectors;
		mesh = new GenerativeIntersectionMesh(edgePoints, edgeVectors, 10);
		colliderMesh = mesh;
		colliderMode = SelectionColliderMode::mesh;
		model = new Model(mesh, &myMat, nullptr);

		addRenderComponent(model);
		addTransformComponent();
	}

	~IntersectionBase() {
		Message<UpdateEvent>::Disconnect(&IntersectionBase::update, this);
		removeRenderComponents();
		removeTransformComponent();

		delete textureC;
		delete model;
		delete mesh;
	}

};