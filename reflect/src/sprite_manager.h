#pragma once

#include "furnace.h"
#include "texture_load.h"

namespace Mirror::Reflect {

struct SpriteRenderer {
	struct Sprite {
		Mat4f model_mat;
		Mat2f tex_coords;
	};

	struct Texture {
		Vk::Image image;
		VkDescriptorSet set;
		std::vector<Sprite> sprite_queue;
	};

	using TextureHandle = usize;
	struct SpriteHandle {
		TextureHandle texture;
		usize index;
	};

	Vk::DescriptorPool descriptor_pool;
	Vk::Pipeline pipeline;
	static constexpr std::array<u16, 6> indices = { 0, 1, 2, 2, 3, 0 };
	Vk::Buffer index_buffer = Vk::Buffer::create({
		.size = sizeof(indices),
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.memory_type = Vk::MemoryType::DeviceLocal,
		});
	std::vector<Texture> textures;

	SpriteRenderer(const u32 max_textures, const VkDescriptorSetLayout vp_set_layout) :
		descriptor_pool{ Vk::DescriptorPool::Builder{}
			.add_set(max_textures)
			.add_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build()
		},
		pipeline{ Vk::Pipeline::createGraphics({
			.vertex_shader = "../../shaders/sprite.vert.spv",
			.fragment_shader =  "../../shaders/sprite.frag.spv",
			.descriptor_layouts = std::array{ vp_set_layout, descriptor_pool.layouts[0] },
			.push_constant = VkPushConstantRange{
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				.offset = 0,
				.size = sizeof(Sprite),
			},
			.render_target = {
				.color_format = Vk::Surface::Format.format,
			},
		}) }
	{
		index_buffer.writeDeviceLocal(indices.data(), sizeof(indices), 0);
		textures.reserve(max_textures);
	}

	Sprite& operator[](const SpriteHandle handle) noexcept { return textures[handle.texture].sprite_queue[handle.index]; }
	Sprite& getSprite(const SpriteHandle& handle) noexcept { return textures[handle.texture].sprite_queue[handle.index]; }

	void cmdRender(const VkCommandBuffer command_buffer, const VkDescriptorSet vp_set) noexcept;
	TextureHandle loadTexture(const std::string_view path, const VkSampler sampler);
	constexpr void queueSprite(const TextureHandle texture, const SpriteRenderer::Sprite& transform) noexcept {
		textures[texture].sprite_queue.push_back(transform);
	}

};

}
