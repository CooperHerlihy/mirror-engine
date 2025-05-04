#pragma once

#include "frame/utils.h"
#include "frame/math.h"

#include <memory>
#include <string_view>
#include <vector>

namespace Mirror {

struct TextureData {
	std::unique_ptr<u8> pixels;
	i32 width;
	i32 height;
	i32 channels;

	TextureData() noexcept : pixels(nullptr), width(0), height(0), channels(0) {}
	TextureData(const std::string_view png_path);

	[[nodiscard]] constexpr bool valid() const {
		return pixels != nullptr && width > 0 && height > 0 && channels > 0;
	}
};

struct ModelData {
	struct Vertex {
		Vec3f position;
		Vec3f normal;
		Vec2f texture_uv;
	};
	std::vector<u32> indices;
	std::vector<Vertex> vertices;

	// TODO: fix index buffer
	ModelData(const std::string_view obj_path, Vec3<bool> flip);

	[[nodiscard]] constexpr bool valid() const {
		return !indices.empty() && !vertices.empty();
	}
};

} // namespace Mirror
