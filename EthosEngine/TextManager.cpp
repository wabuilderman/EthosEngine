#include "TextManager.hpp"

// TODO: Add ability to create gradients
void TextManager::modifyRect(
	Mesh* mesh, size_t index,
	float x, float y, float z,
	float width, float height,
	const glm::vec4& color,
	const FontManager::Font::CharacterInfo::UV_Coords& uv
) {
	mesh->vertices[index * 4 + 0].pos = { x, y, z };
	mesh->vertices[index * 4 + 0].color = color;//{ x, y, z, 1.0f };
	mesh->vertices[index * 4 + 0].texCoord = { uv.left, uv.top };

	mesh->vertices[index * 4 + 1].pos = { x, y - height, z };
	mesh->vertices[index * 4 + 1].color = color;//{ x, y - height, z, 1.0f };
	mesh->vertices[index * 4 + 1].texCoord = { uv.left, uv.bottom };

	mesh->vertices[index * 4 + 2].pos = { x, y, z + width };
	mesh->vertices[index * 4 + 2].color = color;//{ x, y, z + width, 1.0f };
	mesh->vertices[index * 4 + 2].texCoord = { uv.right, uv.top };

	mesh->vertices[index * 4 + 3].pos = { x, y - height, z + width };
	mesh->vertices[index * 4 + 3].color = color;//{ x, y - height, z + width, 1.0f };
	mesh->vertices[index * 4 + 3].texCoord = { uv.right, uv.bottom };
}

void TextManager::addRect(
	Mesh* mesh, 
	float x, float y, float z,
	float width, float height,
	const glm::vec4& color,
	const FontManager::Font::CharacterInfo::UV_Coords& uv
) {
	uint32_t baseIndex = (uint32_t)mesh->vertices.size();
	mesh->vertices.push_back({ {x, y,          z}, {0, 0, 1}, {1, 0, 0}, color/*{x, y,          z, 1.f}*/, {uv.left,  uv.top   } });
	mesh->vertices.push_back({ {x, y - height, z}, {0, 0, 1}, {1, 0, 0}, color/*{x, y - height, z, 1.f}*/, {uv.left,  uv.bottom} });
	mesh->vertices.push_back({ {x, y,          z + width}, {0, 0, 1}, {1, 0, 0}, color/*{x, y,          z + width, 1.f}*/, {uv.right, uv.top   } });
	mesh->vertices.push_back({ {x, y - height, z + width}, {0, 0, 1}, {1, 0, 0}, color/*{x, y - height, z + width, 1.f}*/, {uv.right, uv.bottom} });

	mesh->indices.push_back(baseIndex + 0); // 0
	mesh->indices.push_back(baseIndex + 1); // 1
	mesh->indices.push_back(baseIndex + 2); // 2

	mesh->indices.push_back(baseIndex + 2); // 2
	mesh->indices.push_back(baseIndex + 1); // 1
	mesh->indices.push_back(baseIndex + 3); // 3
}

size_t TextManager::editText(TextModel* model, const FontManager::FontIdentity& fontIdentity, std::string newText, size_t offset) {
	size_t newTextLength = newText.length();
	size_t oldTextLength = model->text.length();

	size_t numNewChars = std::max(model->maxCharacters, newTextLength + offset) - oldTextLength;

	for (size_t i = 0; i < newTextLength - numNewChars; ++i) {
		model->text[offset + i] = newText[i];
	}
	model->text = model->text.append(newText.substr(newTextLength - numNewChars, numNewChars));

	float advance = 0.0f;
	float z = 0.0f;
	float sortingScale = 0.003f;
	float pixelscale = getPixelScale() / ((float) fontIdentity.quality);

	const FontManager::Font& font = FontManager::fonts[fontIdentity];

	glm::vec4 color = { 0,0,0,1 };
	for (size_t i = 0; i < offset && i < model->text.length(); ++i) {
		if (model->text[i] == '\x1D') {
			color.r = ((unsigned char)model->text[i + 1] - 1) / 254.f;
			color.g = ((unsigned char)model->text[i + 2] - 1) / 254.f;
			color.b = ((unsigned char)model->text[i + 3] - 1) / 254.f;
			i += 4;
		}
	}

	for (size_t i = offset; i < model->text.length(); ++i) {
		if (model->text[i] == '\x1D') {
			color.r = ((unsigned char)model->text[i + 1] - 1) / 254.f;
			color.g = ((unsigned char)model->text[i + 2] - 1) / 254.f;
			color.b = ((unsigned char)model->text[i + 3] - 1) / 254.f;
			i += 4;
		}

		// Calculate character info & kerning
		glm::vec2 kerning = { 0.0f, 0.0f };
		if (i > 0) {
			kerning = FontManager::getKerning(model->text[i - 1], model->text[i]) * pixelscale;
			if (!kerning.x && !kerning.y) {
				z = 0.f;
			}
		}

		const auto& infoIter = font.mapping.find(model->text[i]);
		assert(infoIter != font.mapping.cend());
		const FontManager::Font::CharacterInfo& info = infoIter->second;

		if (i < oldTextLength) {
			// Modify mesh
			modifyRect(model->mesh,
				i, z,
				kerning.y + (info.metrics.horiBearingY * pixelscale),
				kerning.x + advance + (info.metrics.horiBearingX * pixelscale),
				info.metrics.width * pixelscale,
				info.metrics.height * pixelscale,
				color,
				info.uv);
		}
		else {
			addRect(model->mesh,
				z,
				kerning.y + (info.metrics.horiBearingY * pixelscale),
				kerning.x + advance + (info.metrics.horiBearingX * pixelscale),
				info.metrics.width * pixelscale,
				info.metrics.height * pixelscale,
				color,
				info.uv);
		}

		advance += ((info.metrics.horiAdvance * pixelscale) + kerning.x);
		z += sortingScale;
	}

	model->width = advance;
	model->height = 0.65f;// ((float)font.size) / 1600.0f;

	return numNewChars;
}

TextModel* TextManager::generateText(std::string text, const FontManager::FontIdentity& fontIdentity, TextModel* oldModel) {

	Mesh* mesh = new Mesh();

	float advance = 0.0f;
	float z = 0.0f;
	float sortingScale = -0.003f;

	float pixelscale = getPixelScale() / ((float) fontIdentity.quality);

	const FontManager::Font& font = FontManager::fonts[fontIdentity];

	float maxHeight = 0.0f;
	glm::vec4 color = { 0,0,0,1 };
	
	for (size_t i = 0; i < text.length(); ++i) {
		if (text[i] == '\x1D') {
			color.r = ((unsigned char)text[i + 1] - 1) / 254.f;
			color.g = ((unsigned char)text[i + 2] - 1) / 254.f;
			color.b = ((unsigned char)text[i + 3] - 1) / 254.f;
			i += 4;
		}

		glm::vec2 kerning = { 0.0f, 0.0f };
		if (i > 0) {
			kerning = FontManager::getKerning(text[i - 1], text[i]) * pixelscale;
			if (!kerning.x && !kerning.y) {
				z = 0.f;
			}
		}

		const auto& infoIter = font.mapping.find(text[i]);
		assert(infoIter != font.mapping.cend());
		const FontManager::Font::CharacterInfo& info = infoIter->second;

		addRect(mesh,
			z,
			kerning.y + (info.metrics.horiBearingY * pixelscale),
			kerning.x + advance + (info.metrics.horiBearingX * pixelscale),
			info.metrics.width * pixelscale,
			info.metrics.height * pixelscale,
			color,
			info.uv);
		advance += ((info.metrics.horiAdvance * pixelscale) + kerning.x);
		z += sortingScale;
	}

	TextModel* model;
	if (oldModel != nullptr) {
		oldModel->~TextModel();
		model = new(oldModel) TextModel(mesh, font.texture);
	}
	else {
		model = new TextModel(mesh, font.texture);
	}

	model->maxCharacters = text.length();
	model->text = text;
	model->width = advance;
		/*(
		(float)model->mesh->vertices[text.length() * 4 - 1].pos.z 
		- (float)model->mesh->vertices[0].pos.z
	) / 64;*/

	model->height = 0.55f;//((float) font.size) / 1600.0f;
	return model;
}