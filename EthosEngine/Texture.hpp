#pragma once
#include "pch.hpp"

#include "ImageResource.hpp"
#include <string>

class Texture {
	Texture();
	
	static constexpr unsigned char defaultTexImage[] = {255, 255, 255, 255};
	static constexpr std::pair<size_t, size_t> defaultTexDim = { 1, 1 };

	void createTexture(const unsigned char * pixels, size_t width, size_t height, bool tiling);

public:

	static void init() {
		defaultTexture = new Texture();
	}

	static Texture* defaultTexture;
	Texture(const unsigned char * pixels, size_t width, size_t height, bool tiling = true);
	Texture(const std::string& path, bool tiling = true);
	
	~Texture();

	uint32_t mipLevels;
	ImageResource* image;
	VkSampler sampler;

	void createTextureSampler(bool doTiling);
};