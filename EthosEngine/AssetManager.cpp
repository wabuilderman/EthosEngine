#include "AssetManager.hpp"
#include "DebugManager.hpp"

#include "RectMesh.hpp"
#include "AxisMesh.hpp"
#include "DefaultMesh.hpp"

#include <stdio.h>
#include <regex>
#include <string>

std::unordered_map<std::string, Mesh*> AssetManager::meshes;
std::unordered_map<std::string, Texture*> AssetManager::textures;
std::unordered_map<std::string, Model*> AssetManager::models;

std::ostream& operator<<(std::ostream& os, const AssetManager::AssetFileEntry& asset) {
	// Write Header
	os << AssetManager::assetFileEntryPrefix;
	size_t numBytes = asset.size();
	os.write(reinterpret_cast<const char*>(&numBytes), sizeof(size_t));

	// Write Type
	os.write(reinterpret_cast<const char*>(&asset.type), sizeof(asset.type));

	// Write Type-Specfic Data
	switch (asset.type) {
	case AssetManager::AssetFileEntry::AssetType::Model:
		os << std::get<AssetManager::AssetFileEntry::ModelDescription>(asset.info);
		break;
	case AssetManager::AssetFileEntry::AssetType::Mesh:
		os << std::get<AssetManager::AssetFileEntry::MeshDescription>(asset.info);
		break;
	case AssetManager::AssetFileEntry::AssetType::Texture:
		os << std::get<AssetManager::AssetFileEntry::TextureDescription>(asset.info);
		break;
	}
	return os;
}

std::istream& operator>>(std::istream& is, AssetManager::AssetFileEntry& asset) {
	// Read Header
	char header[sizeof(AssetManager::assetFileEntryPrefix)];
	is.read(header, sizeof(AssetManager::assetFileEntryPrefix));
	size_t numBytes;
	is.read(reinterpret_cast<char*>(&numBytes), sizeof(size_t));

	// Read Type
	is.read(reinterpret_cast<char*>(&asset.type), sizeof(AssetManager::AssetFileEntry::AssetType));

	// Read Type-Specfic Data
	switch (asset.type) {
	case AssetManager::AssetFileEntry::AssetType::Model:
		is >> std::get<AssetManager::AssetFileEntry::ModelDescription>(asset.info);
		break;
	case AssetManager::AssetFileEntry::AssetType::Mesh:
		is >> std::get<AssetManager::AssetFileEntry::MeshDescription>(asset.info);
		break;
	case AssetManager::AssetFileEntry::AssetType::Texture:
		is >> std::get<AssetManager::AssetFileEntry::TextureDescription>(asset.info);
		break;
	}

	return is;
}

size_t AssetManager::AssetFileEntry::size() const {
	size_t numBytes = sizeof(AssetType);

	switch (type) {
	case AssetType::Model:
		numBytes += std::get<ModelDescription>(info).size();
		break;
	case AssetType::Mesh:
		numBytes += std::get<MeshDescription>(info).size();
		break;
	case AssetType::Texture:
		numBytes += std::get<TextureDescription>(info).size();
		break;
	}

	return numBytes;
}

std::string AssetManager::getFileExtension(const std::string& filepath) {
	static const std::regex ext_regex("[.]([a-z]|[A-Z])+");

	std::smatch match;
	if (std::regex_search(filepath, match, ext_regex)) {
		std::cout << match[0].str() << std::endl;
		return match[0].str();
	}

	return "";
}

void AssetManager::saveAssetFile(const std::string& filename, const std::vector<AssetFileEntry>& assets) {
	std::ofstream ofs(filename, std::ofstream::binary);
	for (const AssetFileEntry& asset : assets) {
		ofs << asset;
	}
	ofs.close();
}

Mesh* AssetManager::AssetFileEntry::MeshDescription::load() {
	if (type == MeshType::Default) {
		if (value.compare("RectMesh") == 0) {
			return RectMesh::get();
		} else if (value.compare("AxisMesh") == 0) {
			return AxisMesh::get();
		} else if (value.compare("DefaultMesh") == 0) {
			return DefaultMesh::get();
		} else {
			DebugManager::message("Invalid Default Mesh Requested.", DebugManager::Priority::ERROR_PRIORITY);
			return nullptr;
		}
	}
	else {
		Mesh* mesh = new Mesh(value);
		meshes.emplace(value, mesh);
		return mesh;
	}
}

Texture* AssetManager::AssetFileEntry::TextureDescription::load() {
	if (type == TextureType::Default) {
		return Texture::defaultTexture;
	} else {
		Texture* tex = new Texture(value);
		textures.emplace(value, tex);
		return tex;
	}
}

Model* AssetManager::AssetFileEntry::ModelDescription::load() {
	Mesh* meshInst = mesh.load();
	Texture* texInst = texture.load();
	Model* modelInst = new Model(meshInst, nullptr, nullptr);

	models.emplace(name, modelInst);
	return modelInst;
}

Model* AssetManager::useModel(const std::string& name) {
	auto it = models.find(name);
	if (it != models.end()) {
		return it->second;
	}
	else {
		DebugManager::message("Unable to find Model: " + name, DebugManager::Priority::ERROR_PRIORITY);
		return nullptr;
	}
}

Mesh* AssetManager::useMesh(const std::string& name) {
	auto it = meshes.find(name);
	if (it != meshes.end()) {
		return it->second;
	}
	else {
		DebugManager::message("Unable to find Mesh: " + name, DebugManager::Priority::ERROR_PRIORITY);
		return nullptr;
	}
}

Texture* AssetManager::useTexture(const std::string& name) {
	auto it = textures.find(name);
	if (it != textures.end()) {
		return it->second;
	}
	else {
		DebugManager::message("Unable to find Texture: " + name, DebugManager::Priority::ERROR_PRIORITY);
		return nullptr;
	}
}

void AssetManager::loadAssetFile(const std::string& filename) {
	std::ifstream ifs(filename);
	std::string line;
	
	if (ifs.is_open()) {
		while (ifs.peek() != std::char_traits<char>::eof()) {
			AssetFileEntry asset;
			ifs >> asset;

			switch (asset.type) {
			case AssetFileEntry::AssetType::Model: {
				AssetFileEntry::ModelDescription& info = std::get<AssetFileEntry::ModelDescription>(asset.info);
				info.load();
			} break;
			case AssetFileEntry::AssetType::Mesh: {
				AssetFileEntry::MeshDescription& info = std::get<AssetFileEntry::MeshDescription>(asset.info);
				info.load();
			} break;
			case AssetFileEntry::AssetType::Texture: {
				AssetFileEntry::TextureDescription& info = std::get<AssetFileEntry::TextureDescription>(asset.info);
				info.load();
			} break;
			}
		}
		ifs.close();
	}
	else {
		DebugManager::message("Unable to open file: " + filename, DebugManager::Priority::ERROR_PRIORITY);
	}
}