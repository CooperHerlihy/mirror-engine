#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "reflect/texture_load.h"

namespace Mirror {

[[nodiscard]] TextureData TextureData::load(const std::string_view path) {
	i32 width, height, channels;
	u8* pixels = stbi_load(path.data(), &width, &height, &channels, STBI_rgb_alpha);
	release_assert(pixels != nullptr);
	release_assert(width > 0);
	release_assert(height > 0);
	release_assert(channels > 0);
	return TextureData{pixels, width, height, channels};
}

void TextureData::free_pixels() noexcept {
	stbi_image_free(pixels);
	pixels = nullptr;
	width = 0;
	height = 0;
	channels = 0;
}

} // namespace Mirror
