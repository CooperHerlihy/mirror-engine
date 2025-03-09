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
		pipeline{ Vk::Pipeline::create_graphics({
			.vertex_shader = "../../shaders/sprite.vert.spv",
			.fragment_shader =  "../../shaders/sprite.frag.spv",
			.descriptor_layouts = std::array{ vp_set_layout, descriptor_pool.layouts[0].handle() },
			.push_constant = VkPushConstantRange{
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				.offset = 0,
				.size = sizeof(Sprite),
			},
			.render_target = {
				.color_format = Vk::SurfaceFormat.format,
				.depth_format = Vk::get_depth_format(),
			},
			.MSAA = VK_SAMPLE_COUNT_4_BIT,
		}) }
	{
		Vk::write_device_local_buffer(index_buffer, indices.data(), sizeof(indices));
		textures.reserve(max_textures);
	}

	void cmd_render(const VkCommandBuffer command_buffer, const VkDescriptorSet vp_set) noexcept;

	using TextureHandle = usize;
	TextureHandle load_texture(const std::string_view path, const VkSampler sampler);
	constexpr void queue_sprite(const TextureHandle texture, const SpriteRenderer::Sprite& transform) noexcept {
		textures[texture].sprite_queue.push_back(transform);
	}

};

}
