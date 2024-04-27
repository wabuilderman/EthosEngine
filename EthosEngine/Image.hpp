#pragma once
#include <vector>

class Image {
public:
	std::vector<unsigned char> buffer;
	size_t height = 0;
	size_t width = 0;
};