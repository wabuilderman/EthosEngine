#include "CurveNodeHandle.hpp"
#include "MyControls.hpp"
#include "RoadBase.hpp"
#include "RoadBuilder.hpp"

void CurveNodeHandle::OnDeselect() {
	myMat.emission = glm::vec4(0.2f, 0.2f, 0.2f, 0.2f);
	if (isChildOfRoadBase) {
		RoadBuilder* rb = (RoadBuilder*)(parent->parent);
		rb->finalizeRoadAlteration(parent->actorID);
	}
}
	

bool CurveNodeHandle::isEndOfRoad() const {
	if (isChildOfRoadBase) {
		if (this->pointIndex == ((RoadBase*)parent)->curveNodeChildIndices.size() - 1 || this->pointIndex == 0) {
			return true;
		}
	}
	return false;
}

void CurveNodeHandle::OnDrag(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) {
	glm::vec3 mouseWorldPos = ObjectSelector::castMouseOntoPlane(Vector::up * 0.3f, Vector::up);
	if (!isnan(mouseWorldPos.x)) {
		teleport(mouseWorldPos + Vector::up * nodeHeight);
		if (isChildOfRoadBase) {
			RoadBase* road = (RoadBase*)parent;
			if (road->roadMesh != nullptr) {
				road->UpdatePoint(this->pointIndex, mouseWorldPos - Vector::up * 0.3f + Vector::up * nodeHeight);
			}

			if (doFreeformDrag && isEndOfRoad()) {
				// First, find the point along the tangent line of the end of the road, that is closest to the endpoint
				// The first control node (farthest of the two from this one), should be placed along the tangent line, a quarter of the way to that point.
				// The second control node should then take that control point's position, make a line from that point to the endpoint.
				// From there, take the point halfway along that new line.
				
				// If this is at the start of a road
				if (this->pointIndex == 0) {
					glm::vec3 startPos = road->getCurveNode(this->pointIndex + 3)->transformComponent->getGlobalTransform().getPosition();
					glm::vec3 startTan = glm::normalize(road->getCurveNode(this->pointIndex + 4)->transformComponent->getGlobalTransform().getPosition() - startPos);

					glm::vec3 control1Pos = startPos + (glm::dot(mouseWorldPos - startPos, startTan) * 0.5f) * startTan;
					glm::vec3 control2Pos = (control1Pos + mouseWorldPos) * 0.5f;

					road->UpdatePoint(this->pointIndex + 2, control1Pos - Vector::up * 0.3f + Vector::up * nodeHeight);
					road->UpdatePoint(this->pointIndex + 1, control2Pos - Vector::up * 0.3f + Vector::up * nodeHeight);
				}
				// If this is at the end of a road
				else {
					glm::vec3 startPos = road->getCurveNode(this->pointIndex - 3)->transformComponent->getGlobalTransform().getPosition();
					glm::vec3 startTan = glm::normalize(road->getCurveNode(this->pointIndex - 4)->transformComponent->getGlobalTransform().getPosition() - startPos);

					glm::vec3 control1Pos = startPos + (glm::dot(mouseWorldPos - startPos, startTan) * 0.5f) * startTan;
					glm::vec3 control2Pos = (control1Pos + mouseWorldPos) * 0.5f;

					road->UpdatePoint(this->pointIndex - 2, control1Pos - Vector::up * 0.3f + Vector::up * nodeHeight);
					road->UpdatePoint(this->pointIndex - 1, control2Pos - Vector::up * 0.3f + Vector::up * nodeHeight);
				}
				
			}
		}
	}
}

void CurveNodeHandle::onKeyPress(KeyPress& e) {
	if (IsSelected()) {
		if (e.keycode == GLFW_KEY_PAGE_UP || e.keycode == GLFW_KEY_PAGE_DOWN) {
			if (e.keycode == GLFW_KEY_PAGE_UP) {
				this->nodeHeight += 0.5f;
				teleport(this->transformComponent->getLocalTransform().getPosition() + Vector::up * 0.5f);
			}
			else {
				this->nodeHeight -= 0.5f;
				teleport(this->transformComponent->getLocalTransform().getPosition() + Vector::down * 0.5f);
			}

			if (this->isChildOfRoadBase) {
				RoadBase* road = (RoadBase*)this->parent;
				if (road->roadMesh != nullptr) {
					road->UpdatePoint(this->pointIndex, this->transformComponent->getLocalTransform().getPosition() - Vector::up * 0.3f);
				}
			}
		}
		else if (e.keycode == GLFW_KEY_MINUS) {
			if (this->isChildOfRoadBase) {
				// Remove the segment of road at the following end, only if this is an end-node
				if (isEndOfRoad()) {

				}
			}
		}
		else if (e.keycode == GLFW_KEY_EQUAL) {
			if (this->isChildOfRoadBase && !ignoreExtendButton) {
				// Extend the road at the following end, only if this is an end-node
				if (isEndOfRoad()) {
					((RoadBase*)this->parent)->ExtendRoad(this->pointIndex == 0);
					this->Deselect();
				}
			}
		}
		
	}
}