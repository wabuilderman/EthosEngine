#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ADVANCES_H

#include <vector>
#include "Image.hpp"
#include "Texture.hpp"
#include <map>

#include "pch.hpp"

class FontManager {
private:
	static FT_Library library;
	static FT_Face face;

public:

	struct CharacterMetrics {
		signed long width;
		signed long height;

		signed long horiBearingX;
		signed long horiBearingY;
		signed long horiAdvance;
	};

	struct FontIdentity {
		enum class Family { Arial } family;
		enum class Quality {
			veryLow = 32,
			low = 64,
			medium = 128,
			high = 256,
			veryHigh = 512,
			ultra = 1024
		} quality;

		FontIdentity() : family(Family::Arial), quality(Quality::medium) {};

		FontIdentity(
			Family family,
			Quality quality
		) : family(family), quality(quality) {};

		static constexpr const char* getFilePath(Family family);

		bool operator<(const FontIdentity& lhr) const {
			return (family < lhr.family) || (quality < lhr.quality);
		}
	};

	// TODO: Maybe fonts are being made in the map w/ default constructor?
	struct Font {
		FontIdentity identity;

		Image image;
		Texture* texture = nullptr;

		struct CharacterInfo {
			FontManager::CharacterMetrics metrics;
			struct UV_Coords {
				float top, bottom, left, right;
			} uv;
		};

		std::map<unsigned char, CharacterInfo> mapping;

		Font(FontIdentity identity = {});
		~Font();

		bool operator<(const Font& lhr) const { return identity < lhr.identity; }
	};

	static std::map<FontIdentity, Font> fonts;

public:
	
	static void init();
	static void shutdown();

	static void setFontSize(unsigned height);
	static glm::vec2 getKerning(unsigned long leftCharcode, unsigned long rightCharcode);
	static void loadFace(const char* path);
	static void processCharacter(unsigned long charcode, Image& image, CharacterMetrics& metrics);

private:
	static void emplaceFont(FontIdentity identity);

};