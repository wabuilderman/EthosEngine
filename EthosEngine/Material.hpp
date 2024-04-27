#pragma once

#include "pch.hpp"
#include "Texture.hpp"

struct Material {
	Texture* diffuseTex = Texture::defaultTexture;
	glm::vec2 diffuseUVScale = { 0, 0 };

	Texture* normalTex = Texture::defaultTexture;
	glm::vec2 normalUVScale = { 0, 0 };

	Texture* displaceTex = Texture::defaultTexture;
	glm::vec2 displaceUVScale = { 0, 0 };

	Texture* specularTex = Texture::defaultTexture;
	glm::vec2 specularUVScale = { 0, 0 };

	Texture* ambientTex = Texture::defaultTexture;
	glm::vec2 ambientUVScale = { 0, 0 };

	glm::vec3 baseColor = { 1.0f, 1.0f, 1.0f };
	glm::vec4 emission = { 0.0f, 0.0f, 0.0f, 0.0f };
	//glm::vec3 subsurfaceRaidus = { 1.0f, 0.2f, 0.1f };
	//glm::vec3 subsurfaceColor = { 0.906f, 0.906f, 0.906f };
	
	//float subsurface = 0.0f;
	float metallic = 0.0f;
	float specular = 0.5f;
	//float specularTint = 0.0f;
	float roughness = 0.4f;
	//float anistropic = 0.0f;
	//float anistropicRotation = 0.0f;
	float sheen = 0.0f;
	//float sheenTint = 0.5f;
	//float clearcoat = 0.0f;
	//float clearcoatRoughness = 0.030f;
	float IOR = 1.450f;
	//float transmission = 0.0f;
	//float transmissionRoughness = 0.0f;
	//float alpha = 1.0f;

	bool isOpaque = true;

	enum class TextureType : unsigned char {
		Color = 1 << 0,
		Alpha = 1 << 1,
		Normal = 1 << 2,
		Displacement = 1 << 3,
		Specular = 1 << 4,
		Ambient = 1 << 5,
	};

	void setTexture(Texture* texture, TextureType usage);
	static Material* get();
};

inline Material::TextureType constexpr operator&(Material::TextureType a, Material::TextureType b) {
	return static_cast<Material::TextureType>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b));
}

inline Material::TextureType constexpr operator|(Material::TextureType a, Material::TextureType b) {
	return static_cast<Material::TextureType>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b));
}