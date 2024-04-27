#pragma once
#include "Node.hpp"
#include "Pose.hpp"

class Armature {
	// Final transform matrix for each bone
	std::vector<glm::mat4> boneMatrices = {};

	// Inverse bind-transforms for each bone
	std::vector<glm::mat4> bindMatrices = {};

	// Transforms for each bone in joint space
	std::vector<glm::mat4> jointMatrices = {};

	// Transforms for each bone in model space
	std::vector<glm::mat4> modelMatrices = {};

	Pose pose;
	bool isDirty = false;

public:
	NodeTree nodeTree;

	inline Transform getBoneTransform(size_t i) {
		return Transform(jointMatrices[i]);
	}

	Transform getBoneModelTransform(size_t i) const {
		const_cast<Armature*>(this)->clean();
		return Transform(modelMatrices[i]);
	}

	inline const Transform& getBoneTransform(size_t i) const {
		return jointMatrices[i];
	}

	static constexpr size_t root = 0;

	Armature() {};

	void addBone(size_t parent, const std::string& name) {
		if (nodeTree.nodes.size() == 0) {
			nodeTree.nodes.push_back(NodeTree::Node(0, 0, name));
			jointMatrices.push_back(glm::mat4(1));
			modelMatrices.push_back(glm::mat4(1));
			bindMatrices.push_back(glm::mat4(1));
			boneMatrices.push_back(glm::mat4(1));
		}
		else {
			clean();
			nodeTree.addNode(parent, name);
			jointMatrices.push_back(glm::translate(glm::mat4(1), glm::vec3(0, 0, 1)));
			modelMatrices.push_back(modelMatrices[parent] * jointMatrices.back());
			bindMatrices.push_back(glm::inverse(modelMatrices.back()));
			boneMatrices.push_back(glm::mat4(1));
		}
	}

	void clean() {
		if (isDirty) {
			modelMatrices[0] = jointMatrices[0];
			boneMatrices[0] =  modelMatrices[0] * bindMatrices[0];

			for (size_t i = 1; i < nodeTree.nodes.size(); ++i) {
				modelMatrices[i] = modelMatrices[nodeTree.nodes[i].parent] * jointMatrices[i];
				boneMatrices[i] = modelMatrices[i] * bindMatrices[i];
			}

			isDirty = false;
		}
	}

	const std::vector<glm::mat4>& getBoneMatrices() const {
		const_cast<Armature*>(this)->clean();
		return boneMatrices;
	}

	static Armature* get() {
		static Armature defaultArmature;
		return &defaultArmature;
	}

	bool isCompatible(const Pose& pose) const {
		return false;
	}

	void setRefPose() {
		isDirty = true;
		clean();

		for (size_t i = 0; i < bindMatrices.size(); ++i) {
			bindMatrices[i] = glm::inverse(modelMatrices[i]);
		}

		isDirty = true;
		clean();
	}

	void applyPose(const Pose& pose) {
		//if (!isCompatible(pose)) {
		//	throw "Incompatible pose!";
		//}

		if (!nodeTree.isCompatible(pose.nodeTree)) {
			throw "Pose doesn't fully match bone structure.";
		}

		for (size_t i = 0; i < pose.nodeTree.nodes.size(); ++i) {
			int index = nodeTree.nodeIndexOf(pose.nodeTree.nodes[i].name);
			jointMatrices[index] = pose.transforms[i].getMatrix();
		}

		//jointMatrices[0] = pose.transforms[0].getMatrix();
		//jointMatrices[1] = pose.transforms[1].getMatrix();
		//jointMatrices[2] = pose.transforms[2].getMatrix();
		isDirty = true;

		//getBone(root).applyPoseNode(pose.root);
	}
};