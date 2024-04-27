
#include "Mesh.hpp"

#include <vector>
#include <iostream>

#include "AssetManager.hpp"
#include "DebugManager.hpp"

#pragma warning(push)
#pragma warning( disable : 6429 26495 26451)
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include "openfbx/ofbx.h"
#pragma warning(pop)

Mesh::Mesh() {

}

void Mesh::loadOBJ(const std::string& path) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
		throw std::runtime_error(warn + err);
	}

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex = {};

			vertex.pos = {
				attrib.vertices[(size_t)index.vertex_index * 3u + 0u],
				attrib.vertices[(size_t)index.vertex_index * 3u + 1u],
				attrib.vertices[(size_t)index.vertex_index * 3u + 2u]
			};

			vertex.texCoord = {
				attrib.texcoords[(size_t)index.texcoord_index * 2u + 0u],
				1.0f - attrib.texcoords[(size_t)index.texcoord_index * 2u + 1u]
			};
			
			// TODO: Read normals
			vertex.normal = { 0.0f, 0.0f, 0.0f };

			vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

			vertices.push_back(vertex);
			indices.push_back((uint32_t)indices.size());
		}
	}
}

Mesh::Mesh(const std::string& path) {
	// get extension
	std::string ext = AssetManager::getFileExtension(path);

	if (ext.compare(".obj") == 0) {
		loadOBJ(path);
	}
	else if (ext.compare(".fbx") == 0) {
		DebugManager::message("For FBX files, please use \"FBXloader\" class", DebugManager::Priority::WARNING_PRIORITY);
	} else {
		DebugManager::message("File extension not recognized: " + ext, DebugManager::Priority::WARNING_PRIORITY);
	}
}