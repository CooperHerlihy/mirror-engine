#pragma once

#include "frame.h"
#include "furnace.h"
#include "sprite_manager.h"

namespace Mirror::Reflect {

struct VPUniform {
	Mat4f view{ 1 };
	Mat4f projection{ 1 };
};

struct Renderer {
	static constexpr u32 MaxFramesInFlight = 2;

	static constexpr VkSurfaceFormatKHR SurfaceFormat{
		.format = VK_FORMAT_B8G8R8A8_SRGB,
		.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
	};

	SDL_Window* window;
	Vk::Surface surface = Vk::create_surface(window);
	Vk::Swapchain swapchain = Vk::Swapchain::create(get_window_size(), surface.handle(), VK_NULL_HANDLE);

	Vk::Image color_image = Vk::Image::create({
		.extent = get_window_extent(),
		.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		.memory_type = Vk::MemoryType::DeviceLocal,
		.format = Vk::SurfaceFormat.format,
		.samples = VK_SAMPLE_COUNT_4_BIT,
		});
	Vk::Image depth_image = Vk::Image::create({
		.extent = get_window_extent(),
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.memory_type = Vk::MemoryType::DeviceLocal,
		.format = Vk::get_depth_format(),
		.aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT,
		.samples = VK_SAMPLE_COUNT_4_BIT,
		});

	u32 current_frame = 0;
	std::array<Vk::Semaphore, MaxFramesInFlight> image_available_semaphores;
	std::array<Vk::Semaphore, MaxFramesInFlight> render_finished_semaphores;
	std::array<Vk::Fence, MaxFramesInFlight> in_flight_fences;
	std::array<VkFence, Vk::Swapchain::MaxImages> images_in_flight = {};
	std::array<Vk::CommandBuffer, MaxFramesInFlight> command_buffers;
	Vk::Sampler nearest_sampler = Vk::create_sampler(VK_FILTER_NEAREST);
	Vk::Sampler linear_sampler = Vk::create_sampler(VK_FILTER_LINEAR);

	Cameraf camera;
	VPUniform vp_data;
	Vk::Buffer vp_buffer = Vk::Buffer::create({
		.size = sizeof(VPUniform),
		.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		.memory_type = Vk::MemoryType::HostVisible,
		});
	Vk::DescriptorPool vp_descriptor_pool = Vk::DescriptorPool::Builder{}
		.add_set()
		.add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.build();
	VkDescriptorSet vp_set = vp_descriptor_pool.allocate_set();

	SpriteRenderer sprite_manager{ 64, vp_descriptor_pool.layouts[0].handle() };

	Renderer(const std::string_view app_name, const Vec2<i32> window_size);
	~Renderer() noexcept {
		Vk::wait_for_fences(images_in_flight);
	}
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&& other) noexcept {
		memcpy(this, &other, sizeof(Renderer));
		memset(&other, 0, sizeof(Renderer));
	}
	Renderer& operator=(Renderer&& other) noexcept {
		if (this == &other) return *this;
		memcpy(this, &other, sizeof(Renderer));
		memset(&other, 0, sizeof(Renderer));
		return *this;
	}

	void update();

	[[nodiscard]] Err resize() {
		Vec2<i32> size = get_window_size();
		if (size.x <= 2 || size.y <= 2) {
			return Err::WindowTooSmall;
		}
		Vk::wait_for_fences(images_in_flight);
		swapchain = Vk::Swapchain::create(size, surface.handle(), swapchain.handle());
		color_image = Vk::Image::create({
			.extent = get_window_extent(),
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			.memory_type = Vk::MemoryType::DeviceLocal,
			.format = Vk::SurfaceFormat.format,
			.samples = VK_SAMPLE_COUNT_4_BIT,
			});
		depth_image = Vk::Image::create({
			.extent = get_window_extent(),
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			.memory_type = Vk::MemoryType::DeviceLocal,
			.format = Vk::get_depth_format(),
			.aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.samples = VK_SAMPLE_COUNT_4_BIT,
			});
		return Err::Success;
	};

	[[nodiscard]] Vec2<i32> get_window_size() const {
		Vec2<i32> size = 0;
		SDL_GetWindowSize(window, &size.x, &size.y);
		return size;
	}
	[[nodiscard]] VkExtent3D get_window_extent() const {
		auto size = get_window_size();
		return { static_cast<u32>(size.x), static_cast<u32>(size.y), 1 };
	}

	void set_projection(const Mat4f& projection) {
		vp_data.projection = projection;
	}

	SpriteRenderer::TextureHandle load_sprite_texture(const std::string_view path, const VkFilter sampler_type = VK_FILTER_LINEAR) {
		VkSampler sampler = (sampler_type == VK_FILTER_NEAREST) ? nearest_sampler.handle() : linear_sampler.handle();
		return sprite_manager.load_texture(path, sampler);
	}

	constexpr void render_sprite(const SpriteRenderer::TextureHandle texture_handle, const SpriteRenderer::Sprite& transform) noexcept {
		sprite_manager.queue_sprite(texture_handle, transform);
	}
};

}
