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
	Vk::Surface surface = Vk::Surface::create(window);
	Vk::Swapchain swapchain = Vk::Swapchain::create(get_window_size(), surface, VK_NULL_HANDLE);
	// depth and color attachments

	u32 current_frame = 0;
	Vk::Semaphore image_available_semaphores[MaxFramesInFlight]{};
	Vk::Semaphore render_finished_semaphores[MaxFramesInFlight]{};
	Vk::Fence in_flight_fences[MaxFramesInFlight]{};
	VkFence images_in_flight[Vk::Swapchain::MaxImages]{};
	Vk::CommandBuffer command_buffers[MaxFramesInFlight]{};
	Vk::Sampler nearest_sampler = Vk::createSampler(VK_FILTER_NEAREST);
	Vk::Sampler linear_sampler = Vk::createSampler(VK_FILTER_LINEAR);

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
	VkDescriptorSet vp_set = vp_descriptor_pool.allocateSet((usize)0);

	SpriteRenderer sprite_manager{ 64, vp_descriptor_pool.layouts[0] };

	Renderer(const std::string_view app_name, const Vec2<i32> window_size);
	~Renderer() noexcept {
		vkQueueWaitIdle(Vk::FurnaceKeeper::queue());
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
		swapchain = Vk::Swapchain::create(size, surface, swapchain.handle);
		// resize depth and color attachments
		return Err::Success;
	};

	[[nodiscard]] Vec2<i32> get_window_size() const {
		Vec2<i32> size = 0;
		SDL_GetWindowSize(window, &size.x, &size.y);
		return size;
	}

	void setProjection(const Mat4f& projection) {
		vp_data.projection = projection;
	}

	SpriteRenderer::TextureHandle loadSpriteTexture(const std::string_view path, const VkFilter sampler_type = VK_FILTER_LINEAR) {
		VkSampler sampler = (sampler_type == VK_FILTER_NEAREST) ? nearest_sampler.handle() : linear_sampler.handle();
		return sprite_manager.loadTexture(path, sampler);
	}

	constexpr void renderSprite(const SpriteRenderer::TextureHandle texture_handle, const SpriteRenderer::Sprite& transform) noexcept {
		sprite_manager.queueSprite(texture_handle, transform);
	}
};

}
