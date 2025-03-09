#include "texture_load.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace Mirror {

[[nodiscard]] TextureData TextureData::load(const char* path) {
	int width, height, channels;
	u8* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels) {
		throw Err::CouldNotOpenFile;
	}
	return TextureData{ pixels, width, height, channels };
}

void TextureData::freePixels() noexcept {
	stbi_image_free(pixels);
	pixels = nullptr;
	width = 0;
	height = 0;
	channels = 0;
}

}

