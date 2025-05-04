#pragma once

#include "frame/frame.h"

#include "reflect/furnace.h"

namespace Mirror::Reflect {

class ModelRenderer {
public:
	using TextureHandle = usize;
	using ModelHandle = usize;

	struct Texture {
		Vk::Image image;
		VkDescriptorSet set;
	};

	using Push = Mat4f;

	struct Model {
		Vk::Buffer index_buffer;
		Vk::Buffer vertex_buffer;
		TextureHandle texture;
		u32 index_count;
	};

	struct ModelTicket {
		Transform3Df transform;
		ModelHandle handle;
	};

	ModelRenderer(const u32 max_textures, const VkDescriptorSetLayout vp_set_layout);

	void cmd_render(const VkCommandBuffer command_buffer, const VkDescriptorSet vp_set) noexcept;

	[[nodiscard]] TextureHandle load_texture(const std::string_view path, const VkSampler sampler);
	[[nodiscard]] ModelHandle load_model(const std::string_view path, const Vec3<bool> flip, const TextureHandle texture);

	constexpr void queue_model(const ModelHandle model, const Transform3Df& transform) noexcept {
		debug_assert(model < m_models.size());
		m_render_queue.emplace_back(transform, model);
	}

private:
	Vk::DescriptorPool m_descriptor_pool;
	Vk::GraphicsPipeline m_pipeline;

	std::vector<Texture> m_textures;
	std::vector<Model> m_models;
	std::vector<ModelTicket> m_render_queue;
};

} // namespace Mirror::Reflect
