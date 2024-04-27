#pragma once
#include "Actor.hpp"
#include "RectMesh.hpp"
#include "RoadBuildButton.hpp"
#include "RoadBase.hpp"
#include "IntersectionBase.hpp"
#include "WorldSpaceMarker.hpp"
#include "IntersectionBuildButton.hpp"

enum class RoadBuilderState { Inactive, BeginRoadPlacement, EndRoadPlacement, BeginIntersectionPlacement};

struct RoadChangeEvent {

};

class RoadBuilder : public Actor {
public:
	Mesh* buttonMesh = nullptr;
	Model* buttonModel = nullptr;
	RoadBuildButton* roadBuildButton = nullptr;
	IntersectionBuildButton* intersectionBuildButton = nullptr;
	RoadBuilderState state = RoadBuilderState::Inactive;

	bool isConnectedToClickEvent = false;
	CurveNodeHandle* tempHandle = nullptr;
	std::vector<ActorID> roads;
	std::vector<ActorID> intersections;

	void update(UpdateEvent& e) {

		if (state == RoadBuilderState::Inactive && isConnectedToClickEvent) {
			isConnectedToClickEvent = false;
			Message<MouseClickEvent>::Disconnect(&RoadBuilder::onClick, this);
		}

		if (state == RoadBuilderState::BeginRoadPlacement && !isConnectedToClickEvent) {
			isConnectedToClickEvent = true;
			Message<MouseClickEvent>::Connect(&RoadBuilder::onClick, this);
		}
	}


	void addRoad(RoadBase* road) {
		addChild(road);
		roads.push_back(road->actorID);
	}
	void addIntersection(IntersectionBase* intersection) {
		addChild(intersection);
		intersections.push_back(intersection->actorID);
	}

	std::pair<ActorID, float> getClosestRoadAndPos(glm::vec3 point) {
		ActorID closestRoadID = -1;
		float closestRoadCurvePos = 0.0f;
		float minDist2 = NAN;

		for (ActorID roadID : roads) {
			RoadBase* road = (RoadBase*)ActorManager::getActorByID(roadID);
			GenerativeCubicBezierMesh* roadMesh = (GenerativeCubicBezierMesh*)road->roadMesh;
			float t = roadMesh->curve.getTClosestToPoint(point);
			glm::vec3 closePoint = roadMesh->curve.interpolate(t);
			float dist2 = glm::length2(point - closePoint);
			if (isnan(minDist2)) {
				minDist2 = dist2;
				closestRoadID = roadID;
				closestRoadCurvePos = t;
			}
			else {
				if (dist2 < minDist2) {
					minDist2 = dist2;
					closestRoadID = roadID;
					closestRoadCurvePos = t;
				}
			}
		}

		return std::make_pair(closestRoadID, closestRoadCurvePos);
	}

	static void findIntersectionsForSegments(const ComplexCubicBezierCurve& curve1, const ComplexCubicBezierCurve& curve2,
		size_t seg1, size_t seg2, std::vector<glm::vec3>& intersections, float maxDist, size_t resolution) {
		// We now have two simple cubic bezier curves. They can intersecct a maximum of 9 times.

		// We will do this by doing a series of line-segment intersection tests, following an approximation of the curve
		glm::vec3 prevPoint1 = curve1.interpolate(((float)seg1) / curve1.getNumSegments());

		for (size_t i = 0; i < resolution; ++i) {
			float p1 = ((float)i + 1) / resolution;
			glm::vec3 curPoint1 = curve1.interpolate(((float)seg1 + p1) / curve1.getNumSegments());

			glm::vec3 prevPoint2 = curve2.interpolate(((float)seg2) / curve2.getNumSegments());
			for (size_t j = 0; j < resolution; ++j) {
				float p2 = ((float)j + 1) / resolution;
				glm::vec3 curPoint2 = curve2.interpolate(((float)seg2 + p2) / curve2.getNumSegments());

				// Ignore Z-axis, as that will be handled seperately
				float denominator = (prevPoint1.x - curPoint1.x) * (prevPoint2.y - curPoint2.y) - (prevPoint1.y - curPoint1.y) * (prevPoint2.x - curPoint2.x);
				if (denominator != 0.f) {
					float t = (prevPoint1.x - prevPoint2.x) * (prevPoint2.y - curPoint2.y) - (prevPoint1.y - prevPoint2.y) * (prevPoint2.x - curPoint2.x) / denominator;
					float u = (prevPoint1.x - prevPoint2.x) * (prevPoint1.y - curPoint1.y) - (prevPoint1.y - prevPoint2.y) * (prevPoint1.x - curPoint1.x) / denominator;

					if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
						intersections.push_back((prevPoint1 + t * (curPoint1 - prevPoint1) + prevPoint2 + u * (curPoint2 - prevPoint2)) * 0.5f);
					}
				}
				prevPoint2 = curPoint2;
			}

			prevPoint1 = curPoint1;
		}


	}

	static void findIntersections(const ComplexCubicBezierCurve& curve1, const ComplexCubicBezierCurve& curve2, std::vector<glm::vec3>& intersections, float maxDist, size_t resolution = 50) {
		for (size_t i = 0; i < curve1.getNumSegments(); ++i) {
			for (size_t j = 0; j < curve2.getNumSegments(); ++j) {
				findIntersectionsForSegments(curve1, curve2, i, j, intersections, maxDist, resolution);
			}
		}

	}


	void finalizeRoadAlteration(ActorID roadID) {

		RoadBase* road = (RoadBase*)ActorManager::getActorByID(roadID);
		GenerativeCubicBezierMesh* roadMesh = (GenerativeCubicBezierMesh*)road->roadMesh;
		/*
		bool isClear = true;
		do {
			for (ActorID otherID : roads) {
				if (otherID == roadID) continue;

				RoadBase* other = (RoadBase*)ActorManager::getActorByID(otherID);
				GenerativeCubicBezierMesh* otherMesh = (GenerativeCubicBezierMesh*)other->roadMesh;

				std::vector<glm::vec3> intersections;
				findIntersections2(otherMesh->curve, roadMesh->curve, intersections, 50.0f, 50);
				if (!intersections.empty()) {
					// find t-pos along both curves
					float otherTPos = otherMesh->curve.getTClosestToPoint(intersections.back());
					float roadTPos = roadMesh->curve.getTClosestToPoint(intersections.back());

					// split both roads at the corresponding positions
					ActorID newOther = other->splitAtTPos(otherTPos);
					ActorID newRoad = road->splitAtTPos(roadTPos);
					
					roads.push_back(newOther);
					roads.push_back(newRoad);



					std::vector<std::pair<ActorID, bool>> points;
					points.push_back(std::pair<ActorID, bool>(otherID, true));
					points.push_back(std::pair<ActorID, bool>(roadID, true));
					points.push_back(std::pair<ActorID, bool>(newOther, false));
					points.push_back(std::pair<ActorID, bool>(newRoad, false));


					addChild(new IntersectionBase(points));
					this->intersections.push_back(children.back()->actorID);
					for (std::pair<ActorID, bool> entry : points) {
						RoadBase* road = (RoadBase*)ActorManager::getActorByID(entry.first);
						if (entry.second) {
							road->startIntersection = children.back()->actorID;
						}
						else {
							road->endIntersection = children.back()->actorID;
						}
						
					}


					isClear = false;
					break;
				}
				else {
					isClear = true;
				}
			}
		} while (!isClear);
		*/
		
	}

	static void findIntersections2(const ComplexCubicBezierCurve& curve1, const ComplexCubicBezierCurve& curve2,
		std::vector<glm::vec3>& intersections, float maxDist, size_t resolution) {

		// First, generate points for each curve.
		std::vector<glm::vec3> points1;
		size_t resolution1 = (resolution)*curve1.getNumSegments();
		points1.reserve(resolution1 + 1);
		for (size_t i = 0; i <= resolution1; ++i) {
			points1.push_back(curve1.interpolate(((float)i) / resolution1));
		}
		std::vector<glm::vec3> points2;
		size_t resolution2 = (resolution)*curve2.getNumSegments();
		points2.reserve(resolution2 + 1);
		for (size_t i = 0; i <= resolution2; ++i) {
			points2.push_back(curve2.interpolate(((float)i) / resolution2));
		}

		// For each point along the first curve, find the spans of points that are close enough
		// If the previous point also had spans that were close enough, merge the spans.
		std::vector<std::pair<std::pair<size_t, size_t>, std::vector<std::pair<size_t, size_t>>>> spans;
		for (size_t i = 0; i <= resolution1; ++i) {
			if (!spans.empty()) {
				if (spans.back().second.empty()) {
					spans.push_back({ {i, 0},{} });
				}
			}
			else {
				spans.push_back({ {i, 0},{} });
			}
			bool inSpan = false;
			size_t start = 0;
			for (size_t j = 0; j <= resolution2; ++j) {
				bool inRange = glm::length2(points1[i] - points2[j]) < maxDist * maxDist;
				if (inRange && !inSpan) {
					inSpan = true;
					start = j;
				}
				else if (!inRange && inSpan) {
					inSpan = false;
					spans.back().second.push_back({ start, j - 1 });
					spans.back().first.second = i;
				}
			}
			if (inSpan) {
				spans.back().second.push_back({ start, resolution2 });
				spans.back().first.second = i;
			}
		}
	}

	void onClick(MouseClickEvent& e) {
		if (state == RoadBuilderState::BeginRoadPlacement) {
			state = RoadBuilderState::EndRoadPlacement;
			children.pop_back();
			RoadBase* newestRoad = new RoadBase(
				tempHandle->transformComponent->getGlobalTransform().getPosition() - Vector::up * 0.3f,
				tempHandle->transformComponent->getGlobalTransform().getPosition() - Vector::up * 0.3f,
				tempHandle);
			addChild(newestRoad);
			roads.push_back(children.back()->actorID);
			newestRoad->getCurveNode(3)->Select();
			tempHandle = nullptr;
			
		}
		else if (state == RoadBuilderState::EndRoadPlacement) {
			state = RoadBuilderState::Inactive;
			
			// Check for intersections:
			RoadBase* newestRoad = (RoadBase*)children.back();

			for (ActorID roadID : roads) {
				RoadBase* road = (RoadBase*)ActorManager::getActorByID(roadID);
				if (road == newestRoad) {
					continue;
				}
				const ComplexCubicBezierCurve& curve1 = ((GenerativeCubicBezierMesh*)road->roadMesh)->curve;
				const ComplexCubicBezierCurve& curve2 = ((GenerativeCubicBezierMesh*)newestRoad->roadMesh)->curve;
				std::vector<glm::vec3> intersections;
				findIntersections(curve1, curve2, intersections, 0.2f, 50);

				/*
				for (const glm::vec3& intersection : intersections) {
					addChild(new WorldSpaceMarker());
					children.back()->teleport(intersection);
				}*/
				
			}

			RoadChangeEvent roadChangeEvent;
			Message<RoadChangeEvent>::Send(roadChangeEvent);

		}
	}

	void onKeyPress(KeyPress& e) {
		if (e.keycode == GLFW_KEY_Z && e.mod == GLFW_MOD_CONTROL) {
			if ((state == RoadBuilderState::Inactive || state == RoadBuilderState::EndRoadPlacement) && roads.size() > 0) {
				ActorManager::scheduleActorDestruction(roads.back());
				for (size_t i = 0; i < children.size(); ++i) {
					if (children[i]->actorID == roads.back()) {
						children.erase(children.begin() + i);
					}
				}
				roads.pop_back();
				RoadChangeEvent roadChangeEvent;
				Message<RoadChangeEvent>::Send(roadChangeEvent);
			}
		}
	}

	RoadBuilder() {
		buttonMesh = new RectMesh();
		buttonModel = new Model(buttonMesh, nullptr, nullptr);

		roadBuildButton = new RoadBuildButton(*this, buttonModel, nullptr, 0.8f, 0.1f, "New Road", [](Actor& owner) {
			RoadBuilder* roadBuilder = (RoadBuilder*)(&owner);
			roadBuilder->state = RoadBuilderState::BeginRoadPlacement;
			roadBuilder->tempHandle = new CurveNodeHandle(nullptr, 0);
			roadBuilder->addChild(roadBuilder->tempHandle);
			roadBuilder->tempHandle->Select();
			roadBuilder->tempHandle->SetDragState(true);
			InputManager::ignoreNextClickRelease();
		});
		roadBuildButton->vertAlign = UIComponent::Alignment::Bottom;
		roadBuildButton->transform.setPosition(glm::vec2(0.f, -1.f));

		intersectionBuildButton = new IntersectionBuildButton(*this, buttonModel, nullptr, 0.8, 0.1f, "New Intersection", [](Actor& owner) {
			RoadBuilder* roadBuilder = (RoadBuilder*)(&owner);
			roadBuilder->state = RoadBuilderState::BeginRoadPlacement;

			roadBuilder->tempHandle = new CurveNodeHandle(nullptr, 0);
			roadBuilder->addChild(roadBuilder->tempHandle);
			roadBuilder->tempHandle->Select();
			roadBuilder->tempHandle->SetDragState(true);
			InputManager::ignoreNextClickRelease();
		});
		intersectionBuildButton->vertAlign = UIComponent::Alignment::Bottom;
		intersectionBuildButton->transform.setPosition(glm::vec2(0.f, -0.8f));
		
		addTransformComponent();
		Message<UpdateEvent>::Connect(&RoadBuilder::update, this);
		Message<KeyPress>::Connect(&RoadBuilder::onKeyPress, this);
	}

	~RoadBuilder(){
		delete roadBuildButton;
		delete intersectionBuildButton;
		delete buttonMesh;
		delete buttonModel;
		Message<UpdateEvent>::Disconnect(&RoadBuilder::update, this);
		Message<KeyPress>::Disconnect(&RoadBuilder::onKeyPress, this);
	}

};