#pragma once
#include "RoadBuilder.hpp"
#include "IntersectionBase.hpp"
#include "VehiclePathTargetHandle.hpp"

class VehiclePath : public Actor {

	

	ActorID roadBuilderID;

	Mesh* curveMesh;
	Model* curveModel;
	Material myMat = Material();

	bool shouldUpdatePath = false;
	float updateTimer = 0.f;

	bool calculatePath(std::vector<std::pair<ActorID, bool>>& path, bool& startDir, bool& targetDir, ActorID target, ActorID start, const std::vector<ActorID>& roads) {
		path.clear();

		std::vector<float> cost;
		std::vector<size_t> prev;
		std::vector<bool> visited;
		std::vector<size_t> direction;

		//std::vector<path>

		cost.resize(roads.size());
		prev.resize(roads.size());
		visited.resize(roads.size());
		direction.resize(roads.size());

		for (size_t i = 0; i < roads.size(); ++i) {
			cost[i] = INFINITY;
			prev[i] = -1;
			visited[i] = false;
			direction[i] = 2;
		}

		size_t current = -1;

		// set the distance to this road as being zero
		for (size_t i = 0; i < roads.size(); ++i) {
			if (roads[i] == target) {
				cost[i] = 0;
				current = i;
				break;
			}
		}

		while (current != -1) {
			RoadBase* currentRoad = (RoadBase*)ActorManager::getActorByID(roads[current]);

			// For each neighbor on the start-side
			if (currentRoad->startIntersection != -1) {
				IntersectionBase* intersection = ((IntersectionBase*)ActorManager::getActorByID(currentRoad->startIntersection));
				for (std::pair<ActorID, bool> roadConnection : intersection->connectedRoads) {
					// find road in roadlist, only if it is unvisited
					for (size_t i = 0; i < roads.size(); ++i) {
						if (roads[i] == roadConnection.first) {
							if (!visited[i]) {
								float tentativeCost = cost[current] + currentRoad->getLength();
								if (tentativeCost < cost[i]) {
									cost[i] = tentativeCost;
									prev[i] = current;
									direction[i] = roadConnection.second;
									direction[current] = false;
								}
							}
						}
					}
				}
			}
			// Repeat for other side of road
			if (currentRoad->endIntersection != -1) {
				IntersectionBase* intersection = ((IntersectionBase*)ActorManager::getActorByID(currentRoad->endIntersection));
				for (std::pair<ActorID, bool> roadConnection : intersection->connectedRoads) {
					// find road in roadlist, only if it is unvisited
					for (size_t i = 0; i < roads.size(); ++i) {
						if (roads[i] == roadConnection.first) {
							if (!visited[i]) {
								float tentativeCost = cost[current] + currentRoad->getLength();
								if (tentativeCost < cost[i]) {
									cost[i] = tentativeCost;
									prev[i] = current;
									direction[i] = roadConnection.second;
									direction[current] = true;
								}
							}
						}
					}
				}
			}

			// Mark the current node as visited
			visited[current] = true;

			// If the current is the target, terminate
			if (roads[current] == start) {
				break;
			}

			// Find the smallest cost & index among unvisited nodes
			float smallestCost = INFINITY;
			size_t smallestIndex = -1;
			for (size_t i = 0; i < roads.size(); ++i) {
				if (!visited[i]) {
					if (cost[i] < smallestCost) {
						smallestIndex = i;
						smallestCost = cost[i];
					}
				}
			}

			current = smallestIndex;
		}

		if (current == -1) {
			return false;
		}

		for (size_t i = 0; i < roads.size(); ++i) {
			if (roads[i] == start) {
				startDir = direction[current];
				size_t j = prev[i];
				// Only form the path if there is a valid path
				if (j == -1) {
					return false;
				}
				else {
					// omit the start and end roads
					while (j != -1) {
						if (prev[j] != -1) {
							path.push_back(std::pair<ActorID, bool>(roads[j], direction[j]));
						}
						else {
							break;
						}
						j = prev[j];
					}


					targetDir = direction[j];
					return true;
				}
			}
		}
		return false;
	}

	void UpdatePath() {
		RoadBuilder* roadBuilder = (RoadBuilder*)ActorManager::getActorByID(roadBuilderID);

		// snap endpoints to nearest point among roads
		std::pair<ActorID, float> startData = roadBuilder->getClosestRoadAndPos(startPos);
		std::pair<ActorID, float> endData = roadBuilder->getClosestRoadAndPos(endPos);

		// TODO: Solve case where both points are along the same curve

		// Use Dijkstra's algorithm to find shortest path (for now, since roads don't have lengths, the 'length' of each road is simply 1)
		std::vector<std::pair<ActorID, bool>> chosenPath;
		bool startDir = false;
		bool targetDir = false;
		bool areConnected = calculatePath(chosenPath, startDir, targetDir, endData.first, startData.first, roadBuilder->roads);

		if (!areConnected) {
			((GenerativeCubicBezierMesh*)curveMesh)->curve = ComplexCubicBezierCurve(glm::vec3(0,0,0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
			((GenerativeCubicBezierMesh*)curveMesh)->calculateMeshData();
			RenderComponentManager::updateMesh(curveMesh);
			return;
		}

		RoadBase* roadStart = (RoadBase*)ActorManager::getActorByID(startData.first);
		RoadBase* roadEnd = (RoadBase*)ActorManager::getActorByID(endData.first);


		// Create the starting road segment
		// TODO: Fix for roads that have intersections on both sides
		ComplexCubicBezierCurve myCurve = !startDir ?
			// if the next road connects at the end of this road
			((GenerativeCubicBezierMesh*)roadStart->roadMesh)->curve.getFirstOfSplit(startData.second)
			:
			// if the next road connects at the start of this road
			((GenerativeCubicBezierMesh*)roadStart->roadMesh)->curve.getSecondOfSplit(startData.second);

		// if the next road connects at the end of this road
		if (!startDir) {
			myCurve.reverse();
		}

		
		// append all roads in the path; also, sort out which way around they go
		RoadBase* lastRoad = roadStart;
		for (std::pair<ActorID, bool> pathNode : chosenPath) {
			ComplexCubicBezierCurve tempCurve = ((GenerativeCubicBezierMesh*)((RoadBase*)ActorManager::getActorByID(pathNode.first))->roadMesh)->curve;
			if (!pathNode.second) {
				tempCurve.reverse();
			}
			myCurve.attachCurveAutomatic(tempCurve);
		}

		// append the ending road segment
		ComplexCubicBezierCurve myCurveEnd = targetDir ?
			((GenerativeCubicBezierMesh*)roadEnd->roadMesh)->curve.getFirstOfSplit(endData.second)
			:
			((GenerativeCubicBezierMesh*)roadEnd->roadMesh)->curve.getSecondOfSplit(endData.second);

		if (!targetDir) {
			myCurveEnd.reverse();
		}

		myCurve.attachCurveAutomatic(myCurveEnd);

		glm::vec3 newStartPos = ((GenerativeCubicBezierMesh*)roadStart->roadMesh)->curve.interpolate(startData.second);
		glm::vec3 newEndPos = ((GenerativeCubicBezierMesh*)roadEnd->roadMesh)->curve.interpolate(endData.second);

		startPos = newStartPos + Vector::up * 0.5f;
		endPos = newEndPos + Vector::up * 0.5f;
		((GenerativeCubicBezierMesh*)curveMesh)->curve = myCurve;
		((GenerativeCubicBezierMesh*)curveMesh)->calculateMeshData();
		RenderComponentManager::updateMesh(curveMesh);
	}

public:
	glm::vec3 startPos = { 2, -2, 0.5f };
	glm::vec3 endPos = { 5, 7.5f, 0.5f };

	void LazyUpdatePath() {
		shouldUpdatePath = true;
	}

	void update(UpdateEvent& e) {
		if (shouldUpdatePath) {
			if (updateTimer < 0.05f) {
				updateTimer += e.dt;
			}
			else {
				updateTimer = 0.0f;
				UpdatePath();
				shouldUpdatePath = false;
			}
		}
	}

	void OnRoadsChanged(RoadChangeEvent& e) {
		// Schedule path update
		LazyUpdatePath();
	}

	

	VehiclePath(ActorID roadBuilderID) : roadBuilderID(roadBuilderID) {

		glm::vec3 initialStartPos = startPos;
		glm::vec3 initialEndPos = endPos;

		ComplexCubicBezierCurve myCurve = ComplexCubicBezierCurve(startPos, startPos, endPos, endPos);
		curveMesh = new GenerativeCubicBezierMesh(myCurve, 500, 0.2f, 0.1f, 0.5f, 0.0f); //true, false, false, false, false, false);
		
		UpdatePath();
		
		curveModel = new Model(curveMesh, &myMat, nullptr);
		addRenderComponent(curveModel);
		addTransformComponent();

		addChild(new VehiclePathTargetHandle(this->actorID, true));
		children.back()->teleport(initialStartPos);
		addChild(new VehiclePathTargetHandle(this->actorID, false));
		children.back()->teleport(initialEndPos);

		Message<UpdateEvent>::Connect(&VehiclePath::update, this);
		Message<RoadChangeEvent>::Connect(&VehiclePath::OnRoadsChanged, this);
	}

	~VehiclePath() {
		removeRenderComponents();

		delete curveMesh;
		delete curveModel;

		Message<UpdateEvent>::Disconnect(&VehiclePath::update, this);
		Message<RoadChangeEvent>::Disconnect(&VehiclePath::OnRoadsChanged, this);
	}
};