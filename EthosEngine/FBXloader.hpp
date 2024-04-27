#pragma once

#include "Mesh.hpp"
#include "openFBX/ofbx.h"
#include "Armature.hpp"
#include <string>
#include <vector>
#include "AnimPose.hpp"

class FBXloader {
private:
	void loadMesh(const ofbx::Mesh* mesh);


public:
	std::vector<Mesh> meshes;
	std::vector<Armature> armatures;
	std::vector<AnimPose> animations;

	struct LoadOptions {
		enum class Axis { X, NEG_X, Y, NEG_Y, Z, NEG_Z };

		float scale = 1.0f;
		Axis forward = Axis::NEG_Z;
		Axis up = Axis::Y;
		Axis bonePrimary = Axis::Y;
		Axis boneSecondary = Axis::X;
	} options;

	void loadFBX(const std::string& path);
};