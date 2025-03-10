#pragma once

#include "frame/frame.h"

namespace Mirror {

struct TextureData {
	u8* pixels = nullptr;
	i32 width = 0;
	i32 height = 0;
	i32 channels = 0;

	TextureData() noexcept {}
	TextureData(u8* pixels, i32 width, i32 height, i32 channels) : pixels(pixels), width(width), height(height), channels(channels) {}
	~TextureData() noexcept { 
		if (pixels) {
			free_pixels();
		}
	}
	TextureData(const TextureData&) = delete;
	TextureData& operator=(const TextureData&) = delete;
	constexpr TextureData(TextureData&& other) noexcept : pixels{ other.pixels }, width{ other.width }, height{ other.height }, channels{ other.channels } { other.pixels = nullptr; }
	TextureData& operator=(TextureData&& other) noexcept { 
		if (this == &other) return *this;
		free_pixels();
		pixels = other.pixels; 
		width = other.width; 
		height = other.height; 
		channels = other.channels; 
		other.pixels = nullptr; 
		other.width = 0;
		other.height = 0;
		other.channels = 0;
		return *this; 
	}

	static TextureData load(const char* path);
	void free_pixels() noexcept;
};

}

