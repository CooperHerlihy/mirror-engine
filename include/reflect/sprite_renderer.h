#pragma once

#include "reflect/furnace.h"
#include "reflect/texture_load.h"

namespace Mirror::Reflect {

class SpriteRenderer {
public:
	struct Sprite {
		Mat4f model_mat;
		Mat2f tex_coords;
	};

	struct Texture {
		Vk::Image image;
		VkDescriptorSet set;
		std::vector<Sprite> sprite_queue;
	};

	SpriteRenderer(const u32 max_textures, const VkDescriptorSetLayout vp_set_layout);

	void cmd_render(const VkCommandBuffer command_buffer, const VkDescriptorSet vp_set) noexcept;
	using TextureHandle = usize;
	TextureHandle load_texture(const std::string_view path, const VkSampler sampler);
	constexpr void queue_sprite(const TextureHandle texture, const SpriteRenderer::Sprite& transform) noexcept {
		debug_assert(texture < m_textures.size());
		m_textures[texture].sprite_queue.push_back(transform);
	}

private:
	Vk::DescriptorPool m_descriptor_pool;
	Vk::GraphicsPipeline m_pipeline;

	static constexpr std::array<u16, 6> Indices = {0, 1, 2, 2, 3, 0};
	Vk::Buffer m_index_buffer = Vk::Buffer::create({
		.size = sizeof(Indices),
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.memory_type = Vk::MemoryType::DeviceLocal,
	});

	std::vector<Texture> m_textures;
};

} // namespace Mirror::Reflect
