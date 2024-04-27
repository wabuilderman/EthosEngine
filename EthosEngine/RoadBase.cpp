#include "RoadBase.hpp"
#include "IntersectionBase.hpp"
#include "RoadBuilder.hpp"

void RoadBase::UpdatePointForCurve(size_t pointIndex, glm::vec3 position, ComplexCubicBezierCurve& curve) {
	// if the index is that of a joint
	if (pointIndex % 3 == 0) {
		// move the joint node to the new position, and keep track of the offset between the two positions
		glm::vec3 offset = position - curve.points[pointIndex];
		curve.points[pointIndex] = position;
		getCurveNode(pointIndex)->transformComponent->accessLocalTransform().setPosition(position + Vector::up * 0.3f);

		if (pointIndex >= 3) {
			glm::vec3 prevJoint = curve.points[pointIndex - 3];
			glm::vec3 vector = (position - prevJoint) / 3.f;

			if (InputManager::localMouseState.buttons[GLFW_MOUSE_BUTTON_RIGHT]) {
				vector = vector;
			}

			// if the control node at index -2 is linear, position it 2/3rds of the way between this joint and the other joint (index -3)
			if (curve.controlModes[pointIndex - 2] == CurveControlMode::linear) {
				curve.points[pointIndex - 2] = prevJoint + vector;
				getCurveNode(pointIndex - 2)->transformComponent->accessLocalTransform().setPosition(curve.points[pointIndex - 2] + Vector::up * 0.3f);
			}

			// if the control node at index -1 is linear, position it 1/3rd of the way between this joint and the other joint (index -3)
			if (curve.controlModes[pointIndex - 1] == CurveControlMode::linear) {
				curve.points[pointIndex - 1] = position - vector;
			}
			else {
				curve.points[pointIndex - 1] += offset;
			}
			getCurveNode(pointIndex - 1)->transformComponent->accessLocalTransform().setPosition(curve.points[pointIndex - 1] + Vector::up * 0.3f);
		}

		if (pointIndex + 3 < curve.points.size()) {
			glm::vec3 nextJoint = curve.points[pointIndex + 3];
			glm::vec3 vector = (position - nextJoint) / 3.f;

			// if the control node at index +2 is linear, position it 2/3rds of the way between this joint and the other joint (index +3)
			if (curve.controlModes[pointIndex + 2] == CurveControlMode::linear) {
				curve.points[pointIndex + 2] = nextJoint + vector;
				getCurveNode(pointIndex + 2)->transformComponent->accessLocalTransform().setPosition(curve.points[pointIndex + 2] + Vector::up * 0.3f);
			}

			// if the control node at index +1 is linear, position it 1/3rd of the way between this joint and the other joint (index +3)
			if (curve.controlModes[pointIndex + 1] == CurveControlMode::linear) {
				curve.points[pointIndex + 1] = position - vector;
			}
			else {
				curve.points[pointIndex + 1] += offset;
			}
			getCurveNode(pointIndex + 1)->transformComponent->accessLocalTransform().setPosition(curve.points[pointIndex + 1] + Vector::up * 0.3f);
		}
	}
	else {
		// move the control node to the new position
		curve.points[pointIndex] = position;
		getCurveNode(pointIndex)->transformComponent->accessLocalTransform().setPosition(position + Vector::up * 0.3f);

		// if the node opposite the adjacent joint from this one (index +2 if index%3==2, else index -2) is smooth, rotate that node such that it remains parallel (and facing the opposite direction) to this node, relative to the joint.
		if (pointIndex % 3 == 2 && pointIndex + 2 < curve.points.size()) {
			if (curve.controlModes[pointIndex + 2] == CurveControlMode::smooth) {
				glm::vec3 joint = curve.points[pointIndex + 1];
				curve.points[pointIndex + 2] = glm::normalize(joint - curve.points[pointIndex]) * glm::length(curve.points[pointIndex + 2] - joint);
				getCurveNode(pointIndex + 2)->transformComponent->accessLocalTransform().setPosition(curve.points[pointIndex + 2] + Vector::up * 0.3f);
			}
		}
		else if (pointIndex % 3 == 1 && pointIndex > 1) {
			if (curve.controlModes[pointIndex - 2] == CurveControlMode::smooth) {
				glm::vec3 joint = curve.points[pointIndex - 1];
				curve.points[pointIndex - 2] = glm::normalize(joint - curve.points[pointIndex]) * glm::length(curve.points[pointIndex - 2] - joint);
				getCurveNode(pointIndex - 2)->transformComponent->accessLocalTransform().setPosition(curve.points[pointIndex - 2] + Vector::up * 0.3f);
			}
		}

		// if this node was previously linear, change its mode to 'free'
		if (curve.controlModes[pointIndex] == CurveControlMode::linear) {
			curve.controlModes[pointIndex] = CurveControlMode::free;
		}
	}
}

void RoadBase::UpdatePoint(size_t pointIndex, glm::vec3 position) {
	UpdatePointForCurve(pointIndex, position, ((GenerativeCubicBezierMesh*)roadMesh)->curve);
	UpdatePointForCurve(pointIndex, position, ((GenerativeCubicBezierMesh*)lineMesh)->curve);

	((GenerativeCubicBezierMesh*)roadMesh)->calculateMeshData();
	((GenerativeCubicBezierMesh*)lineMesh)->calculateMeshData();


	if (startIntersection != (size_t)-1) {
		IntersectionBase* intersection = (IntersectionBase*) ActorManager::getActorByID(startIntersection);
		intersection->updateMesh();
	}

	if (endIntersection != (size_t)-1) {
		IntersectionBase* intersection = (IntersectionBase*)ActorManager::getActorByID(endIntersection);
		intersection->updateMesh();
	}

	RenderComponentManager::updateMesh(roadMesh);
	RenderComponentManager::updateMesh(lineMesh);

	RoadChangeEvent roadChangeEvent;
	Message<RoadChangeEvent>::Send(roadChangeEvent);
}