#include "Material.hpp"

Material* Material::get() {
	static Material defaultMat = Material();
	return &defaultMat;
}

void Material::setTexture(Texture* texture, TextureType usage) {
	if ((bool)(usage & TextureType::Color)) {
		diffuseTex = texture;
		diffuseUVScale = { 1, 1 };
	}

	if ((bool)(usage & TextureType::Alpha)) {
		diffuseTex = texture;
		diffuseUVScale = { 1, 1 };
	}

	if ((bool)(usage & TextureType::Normal)) {
		normalTex = texture;
		normalUVScale = { 1, 1 };
	}

	if ((bool)(usage & TextureType::Displacement)) {
		displaceTex = texture;
		displaceUVScale = { 1, 1 };
	}

	if ((bool)(usage & TextureType::Specular)) {
		specularTex = texture;
		specularUVScale = { 1, 1 };
	}

	if ((bool)(usage & TextureType::Ambient)) {
		ambientTex = texture;
		ambientUVScale = { 1, 1 };
	}
}