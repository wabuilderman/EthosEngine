#include "FontManager.hpp"
#include "TextManager.hpp"

FT_Library FontManager::library = nullptr;
FT_Face FontManager::face = nullptr;

std::map<FontManager::FontIdentity, FontManager::Font> FontManager::fonts;

constexpr const char* FontManager::FontIdentity::getFilePath(Family family) {
	switch (family) {
		case Family::Arial:
			return "fonts/Arial.ttf";
		default:
			return "fonts/Arial.ttf";
	}
}

FontManager::Font::Font(FontIdentity identity) : identity(identity) {

	FontManager::loadFace(FontIdentity::getFilePath(identity.family));

	size_t padding = 1;
	size_t totalSize = (size_t)identity.quality + padding * 2;
	FontManager::setFontSize((unsigned)identity.quality);

	image.width = totalSize * 16;
	image.height = totalSize * 8;
	image.buffer.resize(image.width * image.height * 4);

	char c = 0;
	for (size_t row = 0; row < 8; ++row) {
		for (size_t col = 0; col < 16; ++col) {
			Image character;
			CharacterInfo info;
			FontManager::processCharacter(c, character, info.metrics);

			// copy image into larger bitmap
			for (size_t y = 0; y < character.height; ++y) {
				for (size_t x = 0; x < character.width; ++x) {

					size_t srcIndex = y * character.width + x;
					size_t dstIndex = 
						(row * totalSize + y + padding) * (image.width) 
						+ (col * totalSize + x + padding);

					image.buffer[dstIndex * 4 + 0] = character.buffer[srcIndex * 4 + 0];
					image.buffer[dstIndex * 4 + 1] = character.buffer[srcIndex * 4 + 1];
					image.buffer[dstIndex * 4 + 2] = character.buffer[srcIndex * 4 + 2];
					image.buffer[dstIndex * 4 + 3] = character.buffer[srcIndex * 4 + 3];
				}
			}

			info.uv.top = (row * totalSize) / (float)image.height;
			info.uv.bottom = (row * totalSize + character.height + 2 * padding) / (float)image.height;
			info.uv.left = (col * totalSize) / (float)image.width;
			info.uv.right = (col * totalSize + character.width + 2 * padding) / (float)image.width;

			this->mapping.insert(std::make_pair(c, info));
			c++;
		}
	}

	texture = new Texture(
		image.buffer.data(),
		image.width,
		image.height
	);
}

FontManager::Font::~Font() {
	delete texture;
}

void FontManager::emplaceFont(FontIdentity identity) {
	fonts.emplace(identity, identity);
}

void FontManager::init() {
	FT_Error error = FT_Init_FreeType(&library);
	if (error) {
		throw "Error occurred when initializing FreeType Library";
	}

	emplaceFont({ FontIdentity::Family::Arial, FontIdentity::Quality::veryLow });
	emplaceFont({ FontIdentity::Family::Arial, FontIdentity::Quality::low });
	emplaceFont({ FontIdentity::Family::Arial, FontIdentity::Quality::medium });
	//emplaceFont({ FontIdentity::Family::Arial, FontIdentity::Quality::high });
	//emplaceFont({ FontIdentity::Family::Arial, FontIdentity::Quality::veryHigh });
}

void FontManager::shutdown() {

	fonts.clear();

	if(face) {
		FT_Done_Face(face);
	}

	FT_Done_FreeType(library);
}

void FontManager::setFontSize(unsigned height) {
	FT_Error error = FT_Set_Pixel_Sizes(face, 0, height);
	if (error) {
		throw "Error occurred when setting character size";
	}
}

glm::vec2 FontManager::getKerning(unsigned long leftCharcode, unsigned long rightCharcode) {
	FT_UInt leftGlyph = FT_Get_Char_Index(face, leftCharcode);
	FT_UInt rightGlyph = FT_Get_Char_Index(face, rightCharcode);

	FT_Vector vec;
	FT_Get_Kerning(face, leftGlyph, rightGlyph, FT_KERNING_DEFAULT, &vec);

	return { vec.x, vec.y };
}

void FontManager::loadFace(const char* path) {
	if (face) {
		FT_Done_Face(face);
	}

	FT_Error error = FT_New_Face(library, path, 0, &face);
	if (error) {
		throw "Error occurred when creating face";
	}
}

void FontManager::processCharacter(unsigned long charcode, Image& image, CharacterMetrics& metrics) {
	FT_UInt glyph_index = FT_Get_Char_Index(face, charcode);

	FT_Error error = FT_Load_Glyph(face, glyph_index, FT_LOAD_COLOR);
	if (error) {
		throw "Error occurred when loading glyph";
	}

	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	if (error) {
		throw "Error occurred when rendering glyph";
	}

	metrics.width = face->glyph->metrics.width;
	metrics.height = face->glyph->metrics.height;
	metrics.horiBearingX = face->glyph->metrics.horiBearingX;
	metrics.horiBearingY = face->glyph->metrics.horiBearingY;
	metrics.horiAdvance = face->glyph->metrics.horiAdvance;

	image.width = face->glyph->bitmap.width;
	image.height = face->glyph->bitmap.rows;
	image.buffer.clear();
	image.buffer.reserve(image.width * image.height);

	if (face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
		for (size_t i = 0; i < image.width * image.height; ++i) {
			unsigned char& value = face->glyph->bitmap.buffer[i];

			image.buffer.push_back(value);
			image.buffer.push_back(value);
			image.buffer.push_back(value);
			image.buffer.push_back(value);
		}
	}
	else {
		throw "Error occured due to unknown pixel mode";
	}
}