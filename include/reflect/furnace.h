#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "frame/frame.h"

namespace Mirror::Vk {

inline constexpr std::array RequiredDeviceExtensions = {"VK_KHR_swapchain"};
#ifdef NDEBUG
inline constexpr std::array<const char*, 0> ValidationLayers = {};
#else
inline constexpr std::array ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
#endif
inline constexpr u32 MaxSwapchainImages = 3;

template <typename T, void (*destructor)(T)> class VulkanResource {
public:
	constexpr VulkanResource() noexcept = default;
	constexpr VulkanResource(T handle) noexcept : m_handle(handle) {}
	~VulkanResource() noexcept {
		if (m_handle != VK_NULL_HANDLE) {
			destructor(m_handle);
		}
	}

	[[nodiscard]] constexpr T handle() const {
		return m_handle;
	}
	[[nodiscard]] constexpr T* ptr() {
		return &m_handle;
	}
	[[nodiscard]] constexpr bool valid() const {
		return m_handle != VK_NULL_HANDLE;
	}

	VulkanResource(const VulkanResource&) = delete;
	VulkanResource& operator=(const VulkanResource&) = delete;
	constexpr VulkanResource(VulkanResource&& other) noexcept : m_handle(other.handle()) {
		other.m_handle = VK_NULL_HANDLE;
	}
	VulkanResource& operator=(VulkanResource&& other) noexcept {
		if (this == &other) {
			return *this;
		}
		if (m_handle != VK_NULL_HANDLE) {
			destructor(m_handle);
		}
		m_handle = other.m_handle;
		other.m_handle = VK_NULL_HANDLE;
		return *this;
	}

private:
	T m_handle = VK_NULL_HANDLE;
};

using QueueFamily = u32;

inline struct Furnace {
	VkInstance instance = VK_NULL_HANDLE;
#ifndef NDEBUG
	VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
#endif
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkCommandPool command_pool = VK_NULL_HANDLE;
	VkQueue queue = VK_NULL_HANDLE;
	QueueFamily queue_family = UINT32_MAX;
	bool SDL_initialized = false;
} g_furnace;

class FurnaceKeeper {
public:
	FurnaceKeeper() noexcept = default;
	FurnaceKeeper(const std::string_view app_name, const std::string_view app_version);
	~FurnaceKeeper() noexcept;
};

[[nodiscard]] constexpr VkInstance instance() {
	return g_furnace.instance;
}
#ifndef NDEBUG
[[nodiscard]] constexpr VkDebugUtilsMessengerEXT debug_messenger() {
	return g_furnace.debug_messenger;
}
#endif
[[nodiscard]] constexpr VkPhysicalDevice physical_device() {
	return g_furnace.physical_device;
}
[[nodiscard]] constexpr VkDevice device() {
	return g_furnace.device;
}
[[nodiscard]] constexpr VkCommandPool command_pool() {
	return g_furnace.command_pool;
}
[[nodiscard]] constexpr VkQueue queue() {
	return g_furnace.queue;
}
[[nodiscard]] constexpr QueueFamily queue_family() {
	return g_furnace.queue_family;
}

[[nodiscard]] std::optional<VkFormat> find_supported_format(const std::span<const VkFormat> candidates,
															const VkImageTiling tiling,
															const VkFormatFeatureFlags features) noexcept;
[[nodiscard]] std::optional<u32> find_memory_type_index(const u32 type_filter,
														const VkMemoryPropertyFlags properties) noexcept;
[[nodiscard]] VkFormat get_depth_format();

using Semaphore = VulkanResource<VkSemaphore, [](const VkSemaphore handle) {
	vkDestroySemaphore(device(), handle, nullptr);
}>;

[[nodiscard]] VkSemaphore create_semaphore();

using Fence = VulkanResource<VkFence, [](const VkFence handle) {
	vkDestroyFence(device(), handle, nullptr);
}>;

[[nodiscard]] VkFence create_fence(const VkFenceCreateFlags flags = 0);

inline void wait_for_fences(const std::span<const VkFence> fences) {
	release_assert(vkWaitForFences(device(), static_cast<u32>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX) ==
				   VK_SUCCESS);
}

inline void wait_for_fence(const VkFence fence) {
	wait_for_fences({&fence, 1});
}

using Sampler = VulkanResource<VkSampler, [](const VkSampler handle) {
	vkDestroySampler(device(), handle, nullptr);
}>;

[[nodiscard]] VkSampler create_sampler(const VkFilter type);

enum struct MemoryType {
	HostVisible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	DeviceLocal = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
};

using DeviceMemory = VulkanResource<VkDeviceMemory, [](const VkDeviceMemory handle) {
	vkFreeMemory(device(), handle, nullptr);
}>;

[[nodiscard]] VkDeviceMemory create_device_memory(const VkMemoryRequirements requirements, const MemoryType type);

void write_to_host_visible_memory(const VkDeviceMemory dst, const void* src, const VkDeviceSize size,
							  const VkDeviceSize offset = 0);

struct Buffer {
	VulkanResource<VkBuffer,
				   [](const VkBuffer handle) {
					   vkDestroyBuffer(device(), handle, nullptr);
				   }>
		buffer;
	DeviceMemory memory;

	[[nodiscard]] constexpr VkBuffer handle() const {
		return buffer.handle();
	}
	[[nodiscard]] constexpr VkBuffer* ptr() {
		return buffer.ptr();
	}
	[[nodiscard]] constexpr bool valid() const {
		return buffer.valid() && memory.valid();
	}

	struct CreateInfo {
		VkDeviceSize size;
		VkBufferUsageFlags usage;
		MemoryType memory_type;
	};

	[[nodiscard]] static Buffer create(const CreateInfo create_info);
};

void write_device_local_buffer(const Buffer& dst_buffer, const void* src_data, const VkDeviceSize size,
							   const VkDeviceSize offset = 0);

using ImageView = VulkanResource<VkImageView, [](const VkImageView handle) {
	vkDestroyImageView(device(), handle, nullptr);
}>;

struct Image {
	VulkanResource<VkImage,
				   [](const VkImage handle) {
					   vkDestroyImage(device(), handle, nullptr);
				   }>
		image;
	DeviceMemory memory;
	ImageView view;

	[[nodiscard]] constexpr VkImage handle() const {
		debug_assert(image.handle() != VK_NULL_HANDLE);
		return image.handle();
	}
	[[nodiscard]] constexpr VkImage* ptr() {
		debug_assert(image.ptr() != nullptr);
		return image.ptr();
	}
	[[nodiscard]] constexpr bool valid() const {
		return image.handle() != VK_NULL_HANDLE && memory.handle() != VK_NULL_HANDLE && view.handle() != VK_NULL_HANDLE;
	}

	struct CreateInfo {
		VkExtent3D extent;
		VkImageUsageFlags usage;
		MemoryType memory_type;
		VkFormat format;
		VkImageType image_type = VK_IMAGE_TYPE_2D;
		VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		u32 mip_levels = 1;
		u32 array_layers = 1;
	};

	[[nodiscard]] static Image create(const CreateInfo& create_info);
};

void write_device_local_image(const Image& dst_image, const void* src_data, const VkExtent3D extent,
							  const u32 pixel_alignment, const VkImageLayout final_layout);

using DescriptorSetLayout = VulkanResource<VkDescriptorSetLayout, [](const VkDescriptorSetLayout handle) {
	vkDestroyDescriptorSetLayout(device(), handle, nullptr);
}>;

struct DescriptorPool {
	VulkanResource<VkDescriptorPool,
				   [](VkDescriptorPool handle) {
					   vkDestroyDescriptorPool(device(), handle, nullptr);
				   }>
		pool;
	std::vector<DescriptorSetLayout> layouts;

	[[nodiscard]] constexpr VkDescriptorPool handle() const {
		return pool.handle();
	}
	[[nodiscard]] constexpr VkDescriptorPool* ptr() {
		return pool.ptr();
	}
	[[nodiscard]] constexpr bool valid() const {
		return pool.valid();
	}

	struct SetInfo {
		u32 count = 0;
		std::vector<VkDescriptorSetLayoutBinding> bindings;
	};

	struct CreateInfo {
		std::vector<SetInfo> sets;
	};

	[[nodiscard]] static DescriptorPool create(const CreateInfo& create_info);

	struct Builder {
		CreateInfo create_info;

		[[nodiscard]] constexpr Builder& add_set(const u32 count = 1) noexcept {
			create_info.sets.emplace_back(count);
			return *this;
		}

		[[nodiscard]] constexpr Builder& add_binding(const VkDescriptorType descriptor_type,
													 const VkShaderStageFlags stages,
													 const u32 descriptor_count = 1) noexcept {
			auto& set = create_info.sets.back();
			set.bindings.emplace_back(static_cast<u32>(set.bindings.size()), descriptor_type, descriptor_count, stages,
									  nullptr);
			return *this;
		}

		[[nodiscard]] DescriptorPool build() const {
			DescriptorPool descriptor_pool = create(create_info);
			debug_assert(descriptor_pool.valid());
			return descriptor_pool;
		}
	};

	void allocate_sets(const std::span<const VkDescriptorSetLayout> layouts,
					   const std::span<VkDescriptorSet> out_sets) const;

	[[nodiscard]] VkDescriptorSet allocate_set(const VkDescriptorSetLayout layout) const {
		VkDescriptorSet set = VK_NULL_HANDLE;
		allocate_sets({&layout, 1}, {&set, 1});
		return set;
	}

	[[nodiscard]] VkDescriptorSet allocate_set(const usize layout_index = 0) const {
		return allocate_set(layouts[layout_index].handle());
	}
};

void write_descriptor_set_buffer(const VkDescriptorSet set, const uint32_t binding, const VkBuffer buffer,
								 const VkDeviceSize range, const VkDeviceSize offset = 0) noexcept;
void write_descriptor_set_image(const VkDescriptorSet set, const uint32_t binding, const VkSampler sampler,
								const VkImageView view, const VkImageLayout layout) noexcept;

using ShaderModule = VulkanResource<VkShaderModule, [](VkShaderModule handle) {
	vkDestroyShaderModule(device(), handle, nullptr);
}>;

[[nodiscard]] VkShaderModule create_shader_module(const std::string_view path);

using PipelineLayout = VulkanResource<VkPipelineLayout, [](const VkPipelineLayout handle) {
	vkDestroyPipelineLayout(device(), handle, nullptr);
}>;

struct GraphicsPipeline {
	ShaderModule vertex_shader;
	ShaderModule fragment_shader;
	PipelineLayout layout;
	VulkanResource<VkPipeline,
				   [](const VkPipeline handle) {
					   vkDestroyPipeline(device(), handle, nullptr);
				   }>
		pipeline;

	[[nodiscard]] constexpr VkPipeline handle() const {
		return pipeline.handle();
	}
	[[nodiscard]] constexpr VkPipeline* ptr() {
		return pipeline.ptr();
	}
	[[nodiscard]] constexpr bool valid() const {
		return vertex_shader.valid() && fragment_shader.valid() && layout.valid() && pipeline.valid();
	}

	struct CreateInfo {
		struct RenderTarget {
			// std::vector<VkFormat> color_formats = {VK_FORMAT_UNDEFINED};
			VkFormat color_format = VK_FORMAT_UNDEFINED;
			VkFormat depth_format = VK_FORMAT_UNDEFINED;
			VkFormat stencil_format = VK_FORMAT_UNDEFINED;
		};

		std::string_view vertex_shader;
		std::string_view fragment_shader;
		std::span<const VkVertexInputBindingDescription> binding_descriptions;
		std::span<const VkVertexInputAttributeDescription> attribute_descriptions;
		std::span<const VkDescriptorSetLayout> descriptor_layouts;
		std::optional<VkPushConstantRange> push_constant = {};
		RenderTarget render_target;
		VkSampleCountFlagBits MSAA = VK_SAMPLE_COUNT_1_BIT;
		VkCullModeFlags cull_mode = VK_CULL_MODE_NONE;
	};

	[[nodiscard]] static GraphicsPipeline create_graphics(const CreateInfo& create_info);
};

using CommandBuffer = VulkanResource<VkCommandBuffer, [](const VkCommandBuffer handle) {
	vkFreeCommandBuffers(device(), command_pool(), 1, &handle);
}>;

void allocate_command_buffers(const std::span<VkCommandBuffer> dst_buffers);

[[nodiscard]] inline std::vector<VkCommandBuffer> create_command_buffers(const usize count) {
	std::vector<VkCommandBuffer> buffers{count};
	allocate_command_buffers(buffers);
	return buffers;
}

[[nodiscard]] inline VkCommandBuffer create_command_buffer() {
	VkCommandBuffer buffer = VK_NULL_HANDLE;
	allocate_command_buffers({&buffer, 1});
	return buffer;
}

void cmd_begin(const VkCommandBuffer cmd, const VkCommandBufferUsageFlags flags = 0);
void cmd_end_and_submit_simple(const VkCommandBuffer cmd, const VkFence fence = VK_NULL_HANDLE);
void cmd_set_viewport_and_scissor(const VkCommandBuffer cmd, const VkExtent3D extent) noexcept;

void cmd_copy_buffer_to_buffer(const VkCommandBuffer cmd, const VkBuffer src_buffer, const VkBuffer dst_buffer,
							   const VkDeviceSize size, const VkDeviceSize src_offset = 0,
							   const VkDeviceSize dst_offset = 0) noexcept;
void cmd_copy_buffer_to_image(const VkCommandBuffer cmd, const VkBuffer src_buffer, VkImage dst_image,
							  const VkExtent3D extent) noexcept;
void cmd_insert_image_memory_barrier(const VkCommandBuffer cmd, const VkImage image, const VkImageLayout old_layout,
									 const VkImageLayout new_layout, const VkPipelineStageFlags src_stage,
									 const VkPipelineStageFlags dst_stage, const VkAccessFlags src_access_mask,
									 const VkAccessFlags dst_access_mask,
									 const VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT,
									 const u32 mip_level = 1) noexcept;

constexpr VkSurfaceFormatKHR SurfaceFormat = {
	.format = VK_FORMAT_B8G8R8A8_SRGB,
	.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
};

using Surface = VulkanResource<VkSurfaceKHR, [](const VkSurfaceKHR handle) {
	vkDestroySurfaceKHR(instance(), handle, nullptr);
}>;

[[nodiscard]] VkSurfaceKHR create_surface(SDL_Window* window);

struct Swapchain {
	static constexpr u32 MaxImages = 3;

	VulkanResource<VkSwapchainKHR,
				   [](const VkSwapchainKHR handle) {
					   vkDestroySwapchainKHR(device(), handle, nullptr);
				   }>
		swapchain;
	u32 current_image = 0;
	u32 image_count = 0;
	std::array<VkImage, MaxImages> images = {};
	std::array<ImageView, MaxImages> image_views;

	[[nodiscard]] constexpr VkSwapchainKHR handle() const {
		return swapchain.handle();
	}
	[[nodiscard]] constexpr VkSwapchainKHR* ptr() {
		return swapchain.ptr();
	}
	[[nodiscard]] constexpr bool valid() const {
		if (swapchain.handle() == VK_NULL_HANDLE) {
			return false;
		}
		for (const auto& image : std::span{images.data(), image_count}) {
			if (image == VK_NULL_HANDLE) {
				return false;
			}
		}
		for (const auto& view : std::span{image_views.data(), image_count}) {
			if (!view.valid()) {
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] static Swapchain create(const Vec2<i32> window_size, const VkSurfaceKHR surface,
										  const VkSwapchainKHR old_swapchain);
};

} // namespace Mirror::Vk
