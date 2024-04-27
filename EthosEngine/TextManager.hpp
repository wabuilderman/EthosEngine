#pragma once
#include "FontManager.hpp"
#include "Image.hpp"
#include "Texture.hpp"
#include "Mesh.hpp"
#include "Model.hpp"
#include "UIComponent.hpp"
#include "RenderComponentManager.hpp"
#include <map>
#include <array>

// NOTE: to change text color use control sequence: '\x1D\xRR\xGG\xBB\'
//       with R, G, and B values in the range \x01 to \xFF
//       DO NOT use \x00 at any point in the sequence.

class TextModel : public Model {
	Material textMat;
public:
	size_t maxCharacters = 0;
	std::string text;
	float width = 0.0f;
	float height = 0.0f;

	TextModel(Mesh* mesh, Texture* texture) : Model(mesh, &textMat, nullptr) {
		textMat.setTexture(texture, Material::TextureType::Color | Material::TextureType::Alpha);
		textMat.isOpaque = false;
	}
	~TextModel() {
		delete mesh;
	}
};

class TextManager {
public:

	static void modifyRect(
		Mesh* mesh, size_t index, 
		float x, float y, float z, 
		float width, float height, const glm::vec4& color,
		const FontManager::Font::CharacterInfo::UV_Coords& uv
	);

	static void addRect(
		Mesh* mesh,
		float x, float y, float z,
		float width, float height, const glm::vec4& color,
		const FontManager::Font::CharacterInfo::UV_Coords& uv
	);

	static float constexpr getPixelScale() { return 1.f/64.f; }

	static size_t editText(TextModel* model, const FontManager::FontIdentity& fontIdentity, std::string newText, size_t offset);
	static TextModel* generateText(std::string text, const FontManager::FontIdentity& fontIdentity, TextModel* oldModel = nullptr);
};

class TextComponent : public UIComponent {
	TextModel* textModel = nullptr;
	FontManager::FontIdentity font;

public:
	TextComponent(Actor& owner, UIComponent* parent = nullptr) :
		UIComponent(owner, textModel, parent) {
	}
	TextComponent(Actor& owner, UIComponent* parent, std::string initialText) :
		UIComponent(
			owner,
			TextManager::generateText(initialText, FontManager::FontIdentity(), nullptr),
			parent
		) {
		textModel = (TextModel*) model;
		width = textModel->width;
		height = textModel->height;
	}

	void setFont(const FontManager::FontIdentity& newFont) {
		font = newFont;
		if (textModel != nullptr) {
			TextManager::editText(textModel, font, textModel->text, 0);
			RenderComponentManager::updateUIComponentVBO(this);
			width = textModel->width;
			height = textModel->height;
		}
	}

	void setText(std::string text) {
		if (textModel != nullptr) {
			if (textModel->text.length() > 0) {
				RenderComponentManager::removeUIComponent(this);
				model = nullptr;
			}
		}
		if (text.length() > 0) {
			textModel = TextManager::generateText(text, font, textModel);
			if (textModel) {
				model = textModel;
				RenderComponentManager::addUIComponent(this);
				width = textModel->width;
				height = textModel->height;
			}
		}
		else {
			if (textModel) {
				textModel->width = 0.f;
				width = 0.f;
				textModel->text = "";
			}
		}
	}

	void editText(std::string text, size_t offset) {
		if (!model) {
			setText(text);
			return;
		}

		size_t numNew = TextManager::editText(textModel, font, text, 0);
		width = textModel->width;
		height = textModel->height;

		if (numNew > 0) {
			RenderComponentManager::removeUIComponent(this);
			RenderComponentManager::addUIComponent(this);
		}
		else {
			RenderComponentManager::updateUIComponentVBO(this);
		}
	}

	float getCharOffset(size_t index) {
		float pixelscale = TextManager::getPixelScale() / ((float)this->font.quality);
		const FontManager::Font& font = FontManager::fonts[this->font];
		float advance = 0.f;
		if (index > textModel->text.length()) {
			index = textModel->text.length();
		}
		for (size_t i = 0; i < index; ++i) {
			if (textModel->text[i] == '\x1D') {
				i += 4;
			}

			glm::vec2 kerning = { 0.0f, 0.0f };
			if (i > 0) {
				kerning = FontManager::getKerning(textModel->text[i - 1], textModel->text[i]) * pixelscale;
			}
			const auto& infoIter = font.mapping.find(textModel->text[i]);
			assert(infoIter != font.mapping.cend());
			const FontManager::Font::CharacterInfo& info = infoIter->second;
			advance += ((info.metrics.horiAdvance * pixelscale) + kerning.x);
		}
		return advance;
	}

	~TextComponent() {
		if (textModel != nullptr) {
			delete textModel;
		}
	}
};