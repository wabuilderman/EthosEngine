#pragma once
#include "Pose.hpp"
#include <iterator>
#include <map>
#include <functional>

class AnimPose {
public:
	//using Keyframe = std::pair<float, Pose>;
	//std::map<float, Keyframe, std::greater<float>> keyframes;
	std::vector<Pose> keyframes;

	// Framerate of the animation, in frames per second
	float framerate = 24.0f;

	// Beginning and end times of the animation, in frames
	int begin = 0;
	int end = 0;
	size_t length = 0;

	bool isLooping = true;

	void configAnim(float framerate, int begin, int end) {
		this->framerate = framerate;
		this->begin = begin;
		this->end = end;

		length = (size_t)end - begin + 1;

		keyframes.resize(length);
	}

	// Frametime is the time in relative frames 
	// ie. 23.4 is 40% between frame 23 and frame 24
	// This is independent of the begin and end times of the animation


	// Returns the index of the nearest previous frame
	// If there is no such frame: 
	//    If isLooping is true, this returns the last frame of the animation
	//    If isLooping is flase, this returns the first frame of the animation
	size_t getNearestPreviousFrame(float frametime) {
		if (frametime < begin) {
			if (isLooping) return length - 1;
			else return 0;
		}
		else if (frametime > end) {
			return length - 1;
		}
		
		return (size_t)frametime - begin;
	}

	// Returns the index of the nearest next frame
	// If there is no such frame: 
	//    If isLooping is true, this returns the first frame of the animation
	//    If isLooping is flase, this returns the last frame of the animation
	size_t getNearestNextFrame(float frametime) {
		if (frametime >= end) {
			if (isLooping) return 0;
			else return length - 1;
		}
		else if (frametime < begin) {
			return 0;
		}
		return (size_t)(frametime) - begin + 1;
	}

	void addKeyframeNode(float time, const Pose& pose, size_t boneIndex) {
		unsigned frameIndex = (size_t)(time * framerate) - begin;

		if (frameIndex < 0 || frameIndex > length) {
			throw "Frame out of bounds!";
		}

		if (keyframes[frameIndex].nodeTree.nodes.size() == 0) {
			keyframes[frameIndex].nodeTree.nodes = pose.nodeTree.nodes;
			keyframes[frameIndex].transforms = pose.transforms;

		} else {
			size_t added = keyframes[frameIndex].nodeTree.merge(pose.nodeTree);
			for (size_t j = 0; j < added; ++j) {
				keyframes[frameIndex].transforms.push_back(Transform());
			}
			size_t localIndex = keyframes[frameIndex].nodeTree.nodeIndexOf(pose.nodeTree.nodes[boneIndex].name);
			keyframes[frameIndex].transforms[localIndex] = pose.transforms[boneIndex];
		}
	}

	Pose getPose(float time) {
		float frametime = time * framerate;

		// Loop Logic
		if (isLooping) {
			if (frametime > end + 1) {
				frametime = fmodf(frametime - begin, (float)length) + begin;
			}
			if (frametime < begin) {
				while (frametime < begin) {
					frametime += length;
				}
			}
		}

		float factor = frametime - (int)frametime;

		size_t prevFrame = getNearestPreviousFrame(frametime);
		size_t nextFrame = getNearestNextFrame(frametime);

		return Pose::lerp(keyframes[prevFrame], keyframes[nextFrame], factor);
	}
};

