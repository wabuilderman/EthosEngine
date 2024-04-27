#include "openFBX/ofbx.h"
#include "FBXloader.hpp"
#include <iostream>
#include <fstream>
#include "Transform.hpp"
#include "AnimPose.hpp"
#include <set>
#include <map>
struct TreeElement {
	TreeElement* sibling = nullptr;
	TreeElement* child = nullptr;

	virtual std::string toString() const = 0;

	friend void displayTree(const TreeElement& element, std::vector<bool> indents = {}) {
		std::vector<bool> childIndents = indents;

		for (unsigned i = 0; i < indents.size(); ++i) {
			if (indents[i]) {
				std::cout << "|  ";
			}
			else {
				std::cout << "   ";
			}
		}

		if (element.sibling) {
			std::cout << "|---";
			childIndents.push_back(true);
		}
		else {
			std::cout << "`---";
			childIndents.push_back(false);
		}

		std::cout << element.toString();

		std::cout << std::endl;

		if (element.child) {
			displayTree(*element.child, childIndents);
		}

		if (element.sibling) {
			displayTree(*element.sibling, indents);
		}

		if (!element.sibling && !element.child) {
			for (unsigned i = 0; i < indents.size(); ++i) {
				if (indents[i]) {
					std::cout << "|  ";
				}
				else {
					std::cout << "   ";
				}
			}
			std::cout << std::endl;
		}
	}
};

struct Tree_IElement : TreeElement {
	const ofbx::IElement& element;

	Tree_IElement(const ofbx::IElement& element) : element(element) {
		if(element.getSibling()) {
			sibling = new Tree_IElement(*element.getSibling());
		}

		if (element.getFirstChild()) {
			child = new Tree_IElement(*element.getFirstChild());
		}
	}

	std::string toString() const override {
		char charbuff[200];
		charbuff[0] = '\0';

		element.getID().toString(charbuff);

		return charbuff;
	}
};


std::pair<float&, float&> getSmallestWeight(std::pair<float&, float&> a, std::pair<float&, float&> b) {
	if (a.second < b.second) {
		return a;
	}
	else {
		return b;
	}
}

void FBXloader::loadMesh(const ofbx::Mesh* mesh) {
	meshes.push_back(Mesh());

	const ofbx::Geometry* geom = mesh->getGeometry();

	int vertexCount = geom->getVertexCount();
	int indexCount = geom->getIndexCount();
	meshes.back().vertices.reserve(vertexCount);
	meshes.back().indices.reserve(indexCount);

	const int* geomIndices = geom->getFaceIndices();
	const ofbx::Vec3* geomVertices = geom->getVertices();
	const ofbx::Vec3* geomNormals = geom->getNormals();
	const ofbx::Vec3* geomTangents = geom->getTangents();
	const ofbx::Vec4* geomColors = geom->getColors();
	const ofbx::Vec2* geomUVs = geom->getUVs();

	const ofbx::Skin* skin = geom->getSkin();
	int clusterCount = 0;

	if (skin) {
		clusterCount = skin->getClusterCount();
	}

	for (int j = 0; j < indexCount; ++j) {
		meshes.back().indices.push_back(
			(unsigned int)((geomIndices[j] < 0) ? (-geomIndices[j] - 1) : (geomIndices[j]))
		);
	}

	for (int j = 0; j < vertexCount; ++j) {
		glm::vec3 pos = glm::vec3({ (float)geomVertices[j].x, (float)geomVertices[j].y, (float)geomVertices[j].z }) * options.scale;

		glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
		if (geomNormals) {
			normal = { (float)geomNormals[j].x, (float)geomNormals[j].y, (float)geomNormals[j].z };
		}

		glm::vec3 tangent = { 0.0f, 0.0f, 0.0f };
		if (geomTangents) {
			tangent = { (float)geomTangents[j].x, (float)geomTangents[j].y, (float)geomTangents[j].z };
		}

		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		if (geomColors) {
			color = { (float)geomColors[j].x, (float)geomColors[j].y, (float)geomColors[j].z,  (float)geomColors[j].w };
		}

		glm::vec2 uv = { 0.0f, 0.0f };
		if (geomUVs) {
			uv = { (float)geomUVs[j].x, (float)geomUVs[j].y };
		}

		meshes.back().vertices.push_back({ pos, normal, tangent, color, uv });
	}

	for (int j = 0; j < clusterCount; ++j) {
		const ofbx::Cluster* cluster = skin->getCluster(j);

		int weightsCount = cluster->getWeightsCount();
		if (weightsCount > 0) {
			const double* weights = cluster->getWeights();

			int indiceCount = cluster->getIndicesCount();
			const int* indices = cluster->getIndices();

			for (int k = 0; k < indiceCount; ++k) {
				size_t index = (size_t)((indices[k] < 0) ? (-indices[k] - 1) : (indices[k]));

				glm::vec4& bone_indices = meshes.back().vertices[index].bone_indices;
				//glm::vec4& bone_weights = meshes.back().vertices[index].bone_weights;

				if (bone_indices.x == -1.0f) {
					bone_indices.x = (float)j + (float)weights[k] * 0.99999f;
					//bone_weights.x = (float)weights[k];
				}
				else if (bone_indices.y == -1.0f) {
					bone_indices.y = (float)j + (float)weights[k] * 0.99999f;
					//bone_weights.y = (float)weights[k];
				}
				else if (bone_indices.z == -1.0f) {
					bone_indices.z = (float)j + (float)weights[k] * 0.99999f;
					//bone_weights.z = (float)weights[k];

				}
				else if (bone_indices.w == -1.0f) {
					bone_indices.w = (float)j + (float)weights[k] * 0.99999f;
					//bone_weights.w = (float)weights[k];
				}
				else {
					// select smallest

					glm::vec4 lcl_indices = glm::vec4((int)(bone_indices.x), (int)(bone_indices.y), (int)(bone_indices.z), (int)(bone_indices.w));
					glm::vec4 lcl_weights = bone_indices - lcl_indices;

					std::pair<float&, float&> xPair = std::pair<float&, float&>(lcl_indices.x, lcl_weights.x);
					std::pair<float&, float&> yPair = std::pair<float&, float&>(lcl_indices.y, lcl_weights.y);
					std::pair<float&, float&> zPair = std::pair<float&, float&>(lcl_indices.z, lcl_weights.z);
					std::pair<float&, float&> wPair = std::pair<float&, float&>(lcl_indices.w, lcl_weights.w);
					std::pair<float&, float&> smallest = getSmallestWeight(getSmallestWeight(xPair, yPair), getSmallestWeight(zPair, wPair));
					if(weights[k] > smallest.second) {
						smallest.first = (float)j;
						smallest.second = (float)weights[k] * 0.99999f;
					}
					lcl_weights = glm::normalize(lcl_weights);

					bone_indices = lcl_indices + lcl_weights * 0.99999f;
				}
			}
		}
		

		const ofbx::Object* link = cluster->getLink();
	}

	//for (Vertex& v : meshes.back().vertices) {
	//	if (v.bone_indices.x == -1.0f) {
	//		v.bone_indices.x = 0;
	//	}
	//	if (v.bone_indices.y == -1.0f) {
	//		v.bone_indices.y = 0;
	//	}
	//	if (v.bone_indices.z == -1.0f) {
	//		v.bone_indices.z = 0;
	//	}
	//	if (v.bone_indices.w == -1.0f) {
	//		v.bone_indices.w = 0;
	//	}
	//}

	//std::cout << "loaded mesh" << std::endl;
}

const ofbx::Object* getArmature(const ofbx::Object* bone) {
	char charbuff[200];
	charbuff[0] = '\0';
	((ofbx::NodeAttribute*)bone->node_attribute)->getAttributeType().toString(charbuff);

	if (std::strcmp(charbuff, "Skeleton") == 0) {
		return getArmature(bone->getParent());
	}

	return bone;
}

void addBonesToArmature(Armature& armature, const ofbx::Object* bone, size_t parent) {

	char charbuff[200];
	charbuff[0] = '\0';
	((ofbx::NodeAttribute*)bone->node_attribute)->getAttributeType().toString(charbuff);

	if (std::strcmp(charbuff, "Skeleton") != 0) {
		return;
	}

	armature.addBone(parent, bone->name);
	size_t current = armature.nodeTree.nodes.size() - 1;

	for (const ofbx::Object* child : bone->getChildren()) {
		addBonesToArmature(armature, child, current);
	}
}

void addBoneToPose(Pose& pose, const ofbx::Object* bone, const ofbx::Object* root) {
	if (bone != root) {
		addBoneToPose(pose, bone->getParent(), root);
		pose.addNode(pose.nodeTree.nodeIndexOf(bone->getParent()->name), bone->name);
	}
	else {
		//if (pose.nodeTree.nodes.size() == 0) {
		pose.addRoot(bone->name);
		//}
	}
}

glm::vec3 vecConvert(const ofbx::Vec3& vec) {
	return { (float)vec.x, (float)vec.y, (float)vec.z };
}

void FBXloader::loadFBX(const std::string& path) {
	std::ifstream file(path, std::iostream::in | std::iostream::binary | std::iostream::ate);
	if (file.is_open()) {

		// Allocate a block of memory large enough for the file
		std::streampos size = file.tellg();
		char* memblock = new char[size];

		// Go to the start of the file
		file.seekg(0, std::iostream::beg);

		// Copy the file into memory
		file.read(memblock, size);

		// Close the file
		file.close();

		// Load scene from file
		ofbx::IScene* iscene = ofbx::load(reinterpret_cast<unsigned char*>(memblock), (int)size, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);

		// Free the memory used for the file content
		delete[] memblock;

		int animationCount = iscene->getAnimationStackCount();
		std::cout << animationCount << " animations found in " << path << std::endl;

		const ofbx::Object* iRoot = iscene->getRoot();

		//Tree_IElement treeRoot = Tree_IElement(iRoot->element);
		//displayTree(treeRoot);
		
		std::vector<const ofbx::Object*> armatureRoots = {};

		for (int i = 0; i < animationCount; ++i) {
			const ofbx::AnimationStack* animStack = iscene->getAnimationStack(i);

			if (animStack->node_attribute) {
				std::cout << animStack->node_attribute->name << std::endl;
			}

			animations.push_back(AnimPose());

			std::set<const ofbx::Object*> bones;

			for (int j = 0; animStack->getLayer(j); ++j) {
				const ofbx::AnimationLayer* layer = animStack->getLayer(j);

				for (int k = 0; layer->getCurveNode(k); ++k) {
					const ofbx::Object* bone = layer->getCurveNode(k)->getBone();

					if (bone == nullptr) continue;

					char charbuff[200];
					charbuff[0] = '\0';
					((ofbx::NodeAttribute*)bone->node_attribute)->getAttributeType().toString(charbuff);

					if (std::strcmp(charbuff, "Skeleton") != 0) {
						continue;
					}

					auto iter = bones.find(bone);
					if (iter == bones.end()) {
						bones.emplace(bone);

						const ofbx::Object* armatureRoot = getArmature(bone);
						if (!armatureRoot) {
							throw "Bone not part of armature!";
						}

						int armatureIndex = -1;
						for (int w = 0; w < armatureRoots.size(); ++w) {
							if (armatureRoots[w]->id == armatureRoot->id) {
								armatureIndex = w;
								break;
							}
						}

						if (armatureIndex == -1) {
							armatureRoots.push_back(armatureRoot);

							std::vector<const ofbx::Object*> children = armatureRoot->getChildren();

							if (children.size() != 1) {
								std::cout << "There is more than one root?" << std::endl;
							}
							else {
								Armature armature;

								addBonesToArmature(armature, children[0], 0);

								armatures.push_back(armature);
							}
						}
						else {
							armatureRoot = armatureRoots[armatureIndex];
						}

						Pose pose;
						std::vector<const ofbx::Object*> children = armatureRoot->getChildren();
						addBoneToPose(pose, bone, children[0]);

						int index = pose.nodeTree.nodeIndexOf(bone->name);


						const ofbx::AnimationCurveNode* translationCurveNode = layer->getCurveNode(*bone, "Lcl Translation");
						const ofbx::AnimationCurveNode* scalingCurveNode = layer->getCurveNode(*bone, "Lcl Scaling");
						const ofbx::AnimationCurveNode* rotationCurveNode = layer->getCurveNode(*bone, "Lcl Rotation");

						std::map<float, std::array<glm::vec3, 3>, std::greater<float>> keyframes;

						std::array<glm::vec3, 3> refPos = {
							vecConvert(bone->getLocalTranslation()),vecConvert(bone->getLocalScaling()), vecConvert(bone->getLocalRotation())
						};

						//if(translationCurveNode->getCurve())
						for (int axis = 0; axis < 3; ++axis) {
							if (translationCurveNode) {
								const ofbx::AnimationCurve* translationCurve = translationCurveNode->getCurve(axis);
								if (translationCurve) {
									for (size_t w = 0; w < translationCurve->getKeyCount(); ++w) {
										float time = (float)ofbx::fbxTimeToSeconds(translationCurve->getKeyTime()[w]);
										float value = translationCurve->getKeyValue()[w];
										auto keyIter = keyframes.find(time);

										if (keyIter == keyframes.end()) {
											std::array<glm::vec3, 3> tmpArray;
											auto nextLowest = keyframes.upper_bound(time);
											if (nextLowest == keyframes.end()) {
												tmpArray = refPos;
											}
											else {
												tmpArray = nextLowest->second;
											}

											tmpArray[0][axis] = value;
											keyframes.emplace(time, tmpArray);
										}
										else {
											keyIter->second[0][axis] = value;
										}
									}
								}
							}

							if (scalingCurveNode) {
								const ofbx::AnimationCurve* scalingCurve = scalingCurveNode->getCurve(axis);
								if (scalingCurve) {
									for (size_t w = 0; w < scalingCurve->getKeyCount(); ++w) {
										float time = (float)ofbx::fbxTimeToSeconds(scalingCurve->getKeyTime()[w]);
										float value = scalingCurve->getKeyValue()[w];
										auto keyIter = keyframes.find(time);

										if (keyIter == keyframes.end()) {
											std::array<glm::vec3, 3> tmpArray;
											auto nextLowest = keyframes.upper_bound(time);
											if (nextLowest == keyframes.end()) {
												tmpArray = refPos;
											}
											else {
												tmpArray = nextLowest->second;
											}
											tmpArray[1][axis] = value;
											keyframes.emplace(time, tmpArray);
										}
										else {
											keyIter->second[1][axis] = value;
										}
									}
								}
							}

							if (rotationCurveNode) {
								const ofbx::AnimationCurve* rotationCurve = rotationCurveNode->getCurve(axis);
								if (rotationCurve) {
									for (size_t w = 0; w < rotationCurve->getKeyCount(); ++w) {
										float time = (float)ofbx::fbxTimeToSeconds(rotationCurve->getKeyTime()[w]);
										float value = rotationCurve->getKeyValue()[w];
										auto keyIter = keyframes.find(time);

										if (keyIter == keyframes.end()) {
											std::array<glm::vec3, 3> tmpArray;
											auto nextLowest = keyframes.upper_bound(time);
											if (nextLowest == keyframes.end()) {
												tmpArray = refPos;
											}
											else {
												tmpArray = nextLowest->second;
											}
											tmpArray[2][axis] = value;
											keyframes.emplace(time, tmpArray);
										}
										else {
											keyIter->second[2][axis] = value;
										}
									}
								}
							}
						}

						float minFrame = FLT_MAX;
						float maxFrame = FLT_MIN;
						float framerate = 24.0f;
						for (auto& keyframe : keyframes) {
							if (keyframe.first > maxFrame) {
								maxFrame = keyframe.first;
							}
							if (keyframe.first < minFrame) {
								minFrame = keyframe.first;
							}
						}

						if (minFrame == FLT_MAX || maxFrame == FLT_MIN) {
							throw "Frame boundry error!";
						}

						animations.back().configAnim(framerate, (int)(minFrame * framerate), (int)(maxFrame * framerate));

						for (auto& keyframe : keyframes) {
							glm::vec3 pos = glm::vec3({ keyframe.second[0].x, keyframe.second[0].y, keyframe.second[0].z });

							//float pitch = (bone->getLocalRotation().x - keyframe.second[2].x)/10;

							pose.transforms[index].setPosition({pos.x, pos.y, pos.z});
							pose.transforms[index].setScale({ keyframe.second[1].x, keyframe.second[1].y, keyframe.second[1].z });
							pose.transforms[index].setRotation(
								  glm::angleAxis(glm::radians(keyframe.second[2].z), glm::vec3(0, 0, 1))
								* glm::angleAxis(glm::radians(keyframe.second[2].y), glm::vec3(0, 1, 0))
								* glm::angleAxis(glm::radians(keyframe.second[2].x), glm::vec3(1, 0, 0))
							);
							

							if (index == 0) {
								pose.transforms[index].rotateAbsolute(glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1)));
							}

							animations.back().addKeyframeNode(keyframe.first, pose, index);
						}

					}

					//pose.transforms[index].setPosition()
					//animPose.addKeyframeNode(times[keyIndex], pose, index);

					//animPose.addKeyframeNode()

					//pose.transforms[index].setPosition();
				}
			}
		}

		int meshCount = iscene->getMeshCount();
		meshes.reserve(meshCount);
		for (int i = 0; i < meshCount; ++i) {
			const ofbx::Mesh* mesh = iscene->getMesh(i);
			//const ofbx::Pose* pose = mesh->getPose();
			
 			loadMesh(mesh);
			std::cout << "Mesh " << i << " Loaded" << std::endl;
		}

		//std::vector<Armature> trueArmatures;
		
		for (const ofbx::Object* armatureRoot : armatureRoots) {
			std::cout << armatureRoot->name << std::endl;

			
			
		}

		for (Armature& armature : armatures) {
			std::cout << armature.nodeTree.nodes.size();
		}

		
	}
}