#pragma once

#include "reflect/furnace.h"
#include "reflect/sprite_renderer.h"
#include "reflect/model_renderer.h"

namespace Mirror::Reflect {

struct VPUniform {
	Mat4f view{1};
	Mat4f projection{1};
};

struct Renderer {
	static constexpr u32 MaxFramesInFlight = 2;

	Renderer(const std::string_view app_name, const Vec2<i32> window_size);
	~Renderer() noexcept {
		Vk::wait_for_fences(m_images_in_flight);
	}
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&& other) noexcept {
		memcpy(this, &other, sizeof(Renderer));
		memset(&other, 0, sizeof(Renderer));
	}
	Renderer& operator=(Renderer&& other) noexcept {
		if (this == &other) {
			return *this;
		}
		memcpy(this, &other, sizeof(Renderer));
		memset(&other, 0, sizeof(Renderer));
		return *this;
	}

	void update();

	[[nodiscard]] bool resize();

	[[nodiscard]] Vec2<i32> get_window_size() const {
		Vec2<i32> size;
		SDL_GetWindowSize(m_window, &size.x, &size.y);
		return size;
	}
	[[nodiscard]] VkExtent3D get_window_extent() const {
		auto size = get_window_size();
		return {static_cast<u32>(size.x), static_cast<u32>(size.y), 1};
	}

	void set_projection(const Mat4f& projection) {
		Vk::write_to_host_visible_memory(m_vp_buffer.memory.handle(), &projection, sizeof(projection),
									 offsetof(VPUniform, projection));
	}
	void set_view(const Mat4f& view) {
		Vk::write_to_host_visible_memory(m_vp_buffer.memory.handle(), &view, sizeof(view), offsetof(VPUniform, view));
	}

	SpriteRenderer::TextureHandle load_sprite_texture(const std::string_view path,
													  const VkFilter sampler_type = VK_FILTER_LINEAR) {
		VkSampler sampler = VK_NULL_HANDLE;
		if (sampler_type == VK_FILTER_LINEAR) {
			sampler = m_nearest_sampler.handle();
		} else if (sampler_type == VK_FILTER_NEAREST) {
			sampler = m_linear_sampler.handle();
		}
		debug_assert(sampler != VK_NULL_HANDLE);
		return m_sprite_manager.load_texture(path, sampler);
	}

	SpriteRenderer::TextureHandle load_model_texture(const std::string_view path,
													 const VkFilter sampler_type = VK_FILTER_LINEAR) {
		VkSampler sampler = VK_NULL_HANDLE;
		if (sampler_type == VK_FILTER_LINEAR) {
			sampler = m_nearest_sampler.handle();
		} else if (sampler_type == VK_FILTER_NEAREST) {
			sampler = m_linear_sampler.handle();
		}
		debug_assert(sampler != VK_NULL_HANDLE);
		return m_model_manager.load_texture(path, sampler);
	}

	ModelRenderer::ModelHandle load_model(const std::string_view path, const Vec3<bool> flip,
										  const ModelRenderer::TextureHandle texture) {
		return m_model_manager.load_model(path, flip, texture);
	}

	constexpr void render_sprite(const SpriteRenderer::TextureHandle texture,
								 const SpriteRenderer::Sprite& transform) noexcept {
		m_sprite_manager.queue_sprite(texture, transform);
	}

	constexpr void render_model(const ModelRenderer::ModelHandle model, const Transform3Df& transform) noexcept {
		m_model_manager.queue_model(model, transform);
	}

private:
	SDL_Window* m_window;
	Vk::Surface m_surface = Vk::create_surface(m_window);
	Vk::Swapchain m_swapchain = Vk::Swapchain::create(get_window_size(), m_surface.handle(), VK_NULL_HANDLE);

	Vk::Image m_color_image = Vk::Image::create({
		.extent = get_window_extent(),
		.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		.memory_type = Vk::MemoryType::DeviceLocal,
		.format = Vk::SurfaceFormat.format,
		.samples = VK_SAMPLE_COUNT_4_BIT,
	});
	Vk::Image m_depth_image = Vk::Image::create({
		.extent = get_window_extent(),
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.memory_type = Vk::MemoryType::DeviceLocal,
		.format = Vk::get_depth_format(),
		.aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT,
		.samples = VK_SAMPLE_COUNT_4_BIT,
	});

	u32 m_current_frame = 0;
	std::array<Vk::CommandBuffer, MaxFramesInFlight> m_command_buffers;
	std::array<Vk::Semaphore, MaxFramesInFlight> m_image_available_semaphores;
	std::array<Vk::Semaphore, MaxFramesInFlight> m_render_finished_semaphores;
	std::array<Vk::Fence, MaxFramesInFlight> m_in_flight_fences;
	std::array<VkFence, Vk::Swapchain::MaxImages> m_images_in_flight = {};
	Vk::Sampler m_nearest_sampler = Vk::create_sampler(VK_FILTER_NEAREST);
	Vk::Sampler m_linear_sampler = Vk::create_sampler(VK_FILTER_LINEAR);

	Vk::Buffer m_vp_buffer = Vk::Buffer::create({
		.size = sizeof(VPUniform),
		.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		.memory_type = Vk::MemoryType::HostVisible,
	});
	Vk::DescriptorPool m_vp_descriptor_pool =
		Vk::DescriptorPool::Builder{}
			.add_set()
			.add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();
	VkDescriptorSet m_vp_set = m_vp_descriptor_pool.allocate_set();

	SpriteRenderer m_sprite_manager{64, m_vp_descriptor_pool.layouts[0].handle()};
	ModelRenderer m_model_manager{64, m_vp_descriptor_pool.layouts[0].handle()};
};

} // namespace Mirror::Reflect
