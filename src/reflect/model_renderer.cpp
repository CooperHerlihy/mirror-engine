#include "reflect/model_renderer.h"

namespace Mirror::Reflect {

ModelRenderer::ModelRenderer(const u32 max_textures, const VkDescriptorSetLayout vp_set_layout)
	: m_descriptor_pool{Vk::DescriptorPool::Builder{}
							.add_set(max_textures)
							.add_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
							.build()},
	  m_pipeline{Vk::GraphicsPipeline::create_graphics({
		  .vertex_shader = "../shaders/model.vert.spv",
		  .fragment_shader = "../shaders/model.frag.spv",
		  .binding_descriptions =
			  std::array{
				  VkVertexInputBindingDescription{
					  .binding = 0,
					  .stride = sizeof(ModelData::Vertex),
					  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
				  },
			  },
		  .attribute_descriptions =
			  std::array{
				  VkVertexInputAttributeDescription{
					  .location = 0,
					  .binding = 0,
					  .format = VK_FORMAT_R32G32B32_SFLOAT,
					  .offset = offsetof(ModelData::Vertex, position),
				  },
				  VkVertexInputAttributeDescription{
					  .location = 1,
					  .binding = 0,
					  .format = VK_FORMAT_R32G32B32_SFLOAT,
					  .offset = offsetof(ModelData::Vertex, normal),
				  },
				  VkVertexInputAttributeDescription{
					  .location = 2,
					  .binding = 0,
					  .format = VK_FORMAT_R32G32_SFLOAT,
					  .offset = offsetof(ModelData::Vertex, texture_uv),
				  },
			  },
		  .descriptor_layouts = std::array{vp_set_layout, m_descriptor_pool.layouts[0].handle()},
		  .push_constant =
			  VkPushConstantRange{
				  .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				  .offset = 0,
				  .size = sizeof(Push),
			  },
		  .render_target{
			  .color_format = Vk::SurfaceFormat.format,
			  .depth_format = Vk::get_depth_format(),
		  },
		  .MSAA = VK_SAMPLE_COUNT_4_BIT,
		  .cull_mode = VK_CULL_MODE_BACK_BIT,
	  })} {
	m_textures.reserve(max_textures);

	debug_assert(m_descriptor_pool.valid());
	debug_assert(m_pipeline.valid());
}

void ModelRenderer::cmd_render(const VkCommandBuffer cmd, const VkDescriptorSet vp_set) noexcept {
	debug_assert(cmd != VK_NULL_HANDLE);
	debug_assert(vp_set != VK_NULL_HANDLE);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.handle());

	for (auto& model : m_render_queue) {
		VkDescriptorSet set_arr[] = {vp_set, m_textures[m_models[model.handle].texture].set};
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.layout.handle(), 0, std::size(set_arr),
								set_arr, 0, nullptr);

		const VkDeviceSize offsets[]{0};
		vkCmdBindVertexBuffers(cmd, 0, 1, m_models[model.handle].vertex_buffer.ptr(), offsets);
		vkCmdBindIndexBuffer(cmd, m_models[model.handle].index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

		Push push = model.transform.matrix();
		vkCmdPushConstants(cmd, m_pipeline.layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Push), &push);
		vkCmdDrawIndexed(cmd, m_models[model.handle].index_count, 1, 0, 0, 0);
	}

	m_render_queue.clear();
}

ModelRenderer::TextureHandle ModelRenderer::load_texture(const std::string_view path, const VkSampler sampler) {
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

ModelRenderer::ModelHandle ModelRenderer::load_model(const std::string_view path, const Vec3<bool> flip, const TextureHandle texture) {
	debug_assert(!path.empty());
	debug_assert(texture < m_textures.size());

	ModelData model{path.data(), flip};
	debug_assert(model.valid());

	Vk::Buffer index_buffer = Vk::Buffer::create({
		.size = model.indices.size() * sizeof(model.indices[0]),
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.memory_type = Vk::MemoryType::DeviceLocal,
	});
	debug_assert(index_buffer.valid());
	Vk::write_device_local_buffer(index_buffer, model.indices.data(), model.indices.size() * sizeof(model.indices[0]));

	Vk::Buffer vertex_buffer = Vk::Buffer::create({
		.size = model.vertices.size() * sizeof(model.vertices[0]),
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.memory_type = Vk::MemoryType::DeviceLocal,
	});
	debug_assert(vertex_buffer.valid());
	Vk::write_device_local_buffer(vertex_buffer, model.vertices.data(),
								  model.vertices.size() * sizeof(model.vertices[0]));

	debug_assert(model.indices.size() <= UINT32_MAX);
	m_models.emplace_back(std::move(index_buffer), std::move(vertex_buffer), texture, static_cast<u32>(model.indices.size()));
	return m_models.size() - 1;
}

} // namespace Mirror::Reflect
