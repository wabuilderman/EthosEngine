#pragma once
#include "Mesh.hpp"
#include "Texture.hpp"
#include "Material.hpp"

#include "Armature.hpp"

class Model {
public:
	Mesh* mesh = nullptr;
	Material* material = nullptr;
	Armature* armature = nullptr;

	Model(Mesh* mesh, Material* material, Armature* armature) :
		mesh(mesh),
		material(material ? material : Material::get()),
		armature(armature ? armature : Armature::get()) {}
};