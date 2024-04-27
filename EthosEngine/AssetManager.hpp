#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include <variant>

#include "Mesh.hpp"
#include "Texture.hpp"
#include "Model.hpp"

class AssetManager {
public:
	static constexpr const char assetFileEntryPrefix[] = { 'A','F','E' };

	static std::string getFileExtension(const std::string& filepath);

	struct AssetFileEntry {
		struct MeshDescription {
			enum class MeshType { Default, File } type = MeshType::Default;
			std::string value = "";
			friend std::ostream& operator<<(std::ostream& os, const MeshDescription& mesh) {
				os.write(reinterpret_cast<const char*>(&mesh.type), sizeof(MeshType));
				os << mesh.value << '\0';
				return os;
			}
			friend std::istream& operator>>(std::istream& is, MeshDescription& mesh) {
				is.read(reinterpret_cast<char*>(&mesh.type), sizeof(MeshType));
				std::getline(is, mesh.value, '\0');
				return is;
			}
			size_t size() const {
				return sizeof(type) + value.length() + 1;
			}
			Mesh* load();
		};

		struct TextureDescription {
			enum class TextureType { Default, File } type = TextureType::Default;
			std::string value = "";
			friend std::ostream& operator<<(std::ostream& os, const TextureDescription& texture) {
				os.write(reinterpret_cast<const char*>(&texture.type), sizeof(TextureType));
				os << texture.value << '\0';
				return os;
			}
			friend std::istream& operator>>(std::istream& is, TextureDescription& texture) {
				is.read(reinterpret_cast<char*>(&texture.type), sizeof(TextureType));
				std::getline(is, texture.value, '\0');
				return is;
			}

			size_t size() const {
				return sizeof(type) + value.length() + 1;
			}
			Texture* load();
		};

		struct ModelDescription {
			std::string name;
			MeshDescription mesh = MeshDescription();
			TextureDescription texture = TextureDescription();
			friend std::ostream& operator<<(std::ostream& os, const ModelDescription& model) {
				return os << model.name << '\0' << model.mesh << model.texture;
			}
			friend std::istream& operator>>(std::istream& is, ModelDescription& model) {
				std::getline(is, model.name, '\0');
				return is >> model.mesh >> model.texture;
			}
			size_t size() const {
				return name.length() + 1 + mesh.size() + texture.size();
			}
			Model* load();
		};

		size_t size() const;
		
		enum class AssetType{ Unset, Model, Mesh, Texture } type = AssetType::Unset;
		std::variant<ModelDescription, MeshDescription, TextureDescription> info;

		friend std::ostream& operator<<(std::ostream& os, const AssetFileEntry& asset);
		friend std::istream& operator>>(std::istream& os, AssetFileEntry& asset);
	};

	static std::unordered_map<std::string, Mesh*> meshes;
	static std::unordered_map<std::string, Texture*> textures;
	static std::unordered_map<std::string, Model*> models;
	
	static Model* useModel(const std::string& name);
	static Mesh* useMesh(const std::string& name);
	static Texture* useTexture(const std::string& name);

	static void loadAssetFile(const std::string& filename);
	static void saveAssetFile(const std::string& filename, const std::vector<AssetFileEntry>& assets);
};