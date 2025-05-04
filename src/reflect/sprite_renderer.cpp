#include "reflect/sprite_renderer.h"

namespace Mirror::Reflect {

SpriteRenderer::SpriteRenderer(const u32 max_textures, const VkDescriptorSetLayout vp_set_layout)
	: m_descriptor_pool{Vk::DescriptorPool::Builder{}
							.add_set(max_textures)
							.add_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
							.build()},
	  m_pipeline{Vk::GraphicsPipeline::create_graphics({
		  .vertex_shader = "../shaders/sprite.vert.spv",
		  .fragment_shader = "../shaders/sprite.frag.spv",
		  .descriptor_layouts = std::array{vp_set_layout, m_descriptor_pool.layouts[0].handle()},
		  .push_constant =
			  VkPushConstantRange{
				  .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				  .offset = 0,
				  .size = sizeof(Sprite),
			  },
		  .render_target{
			  .color_format = Vk::SurfaceFormat.format,
			  .depth_format = Vk::get_depth_format(),
		  },
		  .MSAA = VK_SAMPLE_COUNT_4_BIT,
	  })} {
	Vk::write_device_local_buffer(m_index_buffer, Indices.data(), sizeof(Indices));
	m_textures.reserve(max_textures);

	debug_assert(m_descriptor_pool.valid());
	debug_assert(m_pipeline.valid());
	debug_assert(m_index_buffer.valid());
}

void SpriteRenderer::cmd_render(const VkCommandBuffer cmd, const VkDescriptorSet vp_set) noexcept {
	debug_assert(cmd != VK_NULL_HANDLE);
	debug_assert(vp_set != VK_NULL_HANDLE);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.handle());
	vkCmdBindIndexBuffer(cmd, m_index_buffer.handle(), 0, VK_INDEX_TYPE_UINT16);

	for (auto& texture : m_textures) {
		VkDescriptorSet set_arr[] = {vp_set, texture.set};
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.layout.handle(), 0, std::size(set_arr),
								set_arr, 0, nullptr);

		for (auto& sprite : texture.render_queue) {
			vkCmdPushConstants(cmd, m_pipeline.layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Sprite), &sprite);
			vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);
		}

		texture.render_queue.clear();
	}
}

SpriteRenderer::TextureHandle SpriteRenderer::load_texture(const std::string_view path, const VkSampler sampler) {
	debug_assert(!path.empty());
	debug_assert(sampler != VK_NULL_HANDLE);

	TextureData texture{path.data()};
	debug_assert(texture.valid());

	VkExtent3D texture_extent = {static_cast<u32>(texture.width), static_cast<u32>(texture.height), 1};
	Vk::Image image = Vk::Image::create({
		.extent = texture_extent,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.memory_type = Vk::MemoryType::DeviceLocal,
		.format = VK_FORMAT_R8G8B8A8_SRGB,
	});
	debug_assert(image.valid());
	Vk::write_device_local_image(image, texture.pixels.get(), texture_extent, 4,
								 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkDescriptorSet set = m_descriptor_pool.allocate_set();
	debug_assert(set != VK_NULL_HANDLE);
	Vk::write_descriptor_set_image(set, 0, sampler, image.view.handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	m_textures.emplace_back(std::move(image), set);
	return m_textures.size() - 1;
}

} // namespace Mirror::Reflect
