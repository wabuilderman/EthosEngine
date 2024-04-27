#pragma once
#include "Transform.hpp"
#include <vector>
#include "Node.hpp"

struct Pose {
	NodeTree nodeTree;
	std::vector<Transform> transforms;

	Pose() {}

	Pose(const Pose& pose) {
		nodeTree = pose.nodeTree;
		transforms = pose.transforms;
	}

	void addRoot(const std::string& name) {
		transforms.push_back(glm::mat4(1));
		nodeTree.nodes.push_back(NodeTree::Node(0, 0, name));
	}

	void addNode(size_t parent, const std::string& name) {
		transforms.push_back(glm::mat4(1));
		nodeTree.addNode(parent, name);
	}

	static Pose lerp(const Pose& a, const Pose& b, float t) {
		if (!a.nodeTree.matches(b.nodeTree)) {
			throw "Error - Interpolation between mismatching poses is not supported.";
		}

		Pose pose(a);

		// Potential performance bottleneck
		for (size_t i = 0; i < a.transforms.size(); ++i) {
			pose.transforms[i] = Transform::lerp(a.transforms[i], b.transforms[i], t).getMatrix();
		}

		return pose;
	}
};