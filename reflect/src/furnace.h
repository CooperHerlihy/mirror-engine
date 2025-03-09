#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "frame.h"

namespace Mirror::Vk {

struct Furnace {
	using QueueFamily = u32;

	static constexpr u32 MaxSwapchainImages = 3;

	static constexpr std::array RequiredDeviceExtensions = { "VK_KHR_swapchain" };
#ifdef NDEBUG
	static constexpr std::array<const char*, 0> ValidationLayers = {};
#else
	static constexpr std::array ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
#endif

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
};

class FurnaceKeeper {
public:
	FurnaceKeeper() noexcept { };
	FurnaceKeeper(const std::string_view app_name, const std::string_view app_version);
	~FurnaceKeeper() noexcept;

	[[nodiscard]] static constexpr Furnace& get() {
		static Furnace furnace;
		return furnace;
	}
};

[[nodiscard]] constexpr VkInstance instance() {
	return FurnaceKeeper::get().instance;
}
[[nodiscard]] constexpr VkDebugUtilsMessengerEXT debugMessenger() {
	return FurnaceKeeper::get().debug_messenger;
}
[[nodiscard]] constexpr VkPhysicalDevice physicalDevice() {
	return FurnaceKeeper::get().physical_device;
}
[[nodiscard]] constexpr VkDevice device() {
	return FurnaceKeeper::get().device;
}
[[nodiscard]] constexpr VkCommandPool commandPool() {
	return FurnaceKeeper::get().command_pool;
}
[[nodiscard]] constexpr VkQueue queue() {
	return FurnaceKeeper::get().queue;
}
[[nodiscard]] constexpr Furnace::QueueFamily queueFamily() {
	return FurnaceKeeper::get().queue_family;
}

[[nodiscard]] std::optional<VkFormat> find_supported_format(std::span<VkFormat> candidates, const VkImageTiling tiling, const VkFormatFeatureFlags features) noexcept;
[[nodiscard]] std::optional<u32> find_memory_type_index(const u32 type_filter, const VkMemoryPropertyFlags properties) noexcept;
[[nodiscard]] VkFormat get_depth_format();

template<typename T, void (*destructor)(T)>
class VulkanResource {
public:
	[[nodiscard]] constexpr T handle() const {
		return handle_;
	}

	[[nodiscard]] constexpr T* ptr() {
		return &handle_;
	}

	~VulkanResource() noexcept {
		if (handle_ != VK_NULL_HANDLE) {
			destructor(handle_);
		}
	}
	constexpr VulkanResource() noexcept { }
	constexpr VulkanResource(T handle) : handle_(handle) { }
	VulkanResource(const VulkanResource&) = delete;
	VulkanResource& operator=(const VulkanResource&) = delete;
	constexpr VulkanResource(VulkanResource&& other) noexcept : handle_(other.handle()) {
		other.handle_ = VK_NULL_HANDLE;
	}
	VulkanResource& operator=(VulkanResource&& other) noexcept {
		if (this == &other) return *this;
		if (handle_ != VK_NULL_HANDLE) {
			destructor(handle_);
		}
		handle_ = other.handle_;
		other.handle_ = VK_NULL_HANDLE;
		return *this;
	}
private:
	T handle_ = VK_NULL_HANDLE;
};

using Semaphore = VulkanResource<VkSemaphore, [](VkSemaphore handle) {
	vkDestroySemaphore(device(), handle, nullptr);
}>;

[[nodiscard]] VkSemaphore createSemaphore();

using Fence = VulkanResource<VkFence, [](VkFence handle) {
	if (handle != VK_NULL_HANDLE) {
		vkDestroyFence(device(), handle, nullptr);
	}
}>;

[[nodiscard]] VkFence createFence(VkFenceCreateFlags flags = 0);

inline void waitForFences(const std::span<Fence> fences) {
	if (vkWaitForFences(Vk::device(), static_cast<u32>(fences.size()), fences.data()->ptr(), VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
		throw Err::Vulkan;
	}
}

inline void waitForFence(Fence& fence) {
	waitForFences({ &fence, 1 });
}

using Sampler = VulkanResource<VkSampler, [](VkSampler handle) {
	vkDestroySampler(device(), handle, nullptr);
}>;

[[nodiscard]] VkSampler createSampler(VkFilter type);

enum struct MemoryType {
	HostVisible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	DeviceLocal = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
};

using DeviceMemory = VulkanResource<VkDeviceMemory, [](VkDeviceMemory handle) {
	vkFreeMemory(Vk::device(), handle, nullptr);
}>;

[[nodiscard]] VkDeviceMemory createDeviceMemory(VkMemoryRequirements requirements, MemoryType type);

struct Buffer {
	VulkanResource<VkBuffer, [](VkBuffer handle) {
		vkDestroyBuffer(Vk::device(), handle, nullptr);
	}> buffer;
	DeviceMemory memory;

	[[nodiscard]] constexpr VkBuffer handle() const {
		return buffer.handle();
	}
	[[nodiscard]] constexpr VkBuffer* ptr() {
		return buffer.ptr();
	}

	struct CreateInfo {
		VkDeviceSize size;
		VkBufferUsageFlags usage;
		MemoryType memory_type;
	};

	[[nodiscard]] static Buffer create(const CreateInfo& create_info);

	void writeHostVisible(const void* data, const VkDeviceSize size, const VkDeviceSize offset = 0) const;
	void writeDeviceLocal(const void* data, const VkDeviceSize size, const VkDeviceSize offset = 0) const;
};

using ImageView = VulkanResource<VkImageView, [](VkImageView handle) {
	vkDestroyImageView(Vk::device(), handle, nullptr);
}>;

struct Image {
	VulkanResource <VkImage, [](VkImage handle) {
		vkDestroyImage(Vk::device(), handle, nullptr);
	}> image;
	DeviceMemory memory;
	ImageView view;

	[[nodiscard]] constexpr VkImage handle() const {
		return image.handle();
	}
	[[nodiscard]] constexpr VkImage* ptr() {
		return image.ptr();
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

	void writeHostVisible(const void* data, const VkExtent3D extent, const u32 pixel_alignment) const;
	void writeDeviceLocal(const void* data, const VkExtent3D extent, const u32 pixel_alignment, const VkImageLayout final_layout) const;
};

using DescriptorSetLayout = VulkanResource<VkDescriptorSetLayout, [](VkDescriptorSetLayout handle) {
	vkDestroyDescriptorSetLayout(device(), handle, nullptr);
}>;

struct DescriptorPool {
	VulkanResource < VkDescriptorPool, [](VkDescriptorPool handle) {
		vkDestroyDescriptorPool(device(), handle, nullptr);
	}> pool;
	std::vector<DescriptorSetLayout> layouts;

	[[nodiscard]] constexpr VkDescriptorPool handle() const {
		return pool.handle();
	}
	[[nodiscard]] constexpr VkDescriptorPool* ptr() {
		return pool.ptr();
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

		[[nodiscard]] constexpr Builder& add_set(u32 count = 1) noexcept {
			create_info.sets.emplace_back(count);
			return *this;
		}

		[[nodiscard]] constexpr Builder& add_binding(VkDescriptorType descriptor_type, VkShaderStageFlags stages, u32 descriptor_count = 1) noexcept {
			auto& set = create_info.sets.back();
			set.bindings.emplace_back(static_cast<u32>(set.bindings.size()), descriptor_type, descriptor_count, stages, nullptr);
			return *this;
		}

		[[nodiscard]] DescriptorPool build() const {
			return create(create_info);
		}
	};

	void allocateSets(const std::span<const VkDescriptorSetLayout> layouts, const std::span<VkDescriptorSet> out_sets) const;

	[[nodiscard]] VkDescriptorSet allocateSet(const VkDescriptorSetLayout layout) const {
		VkDescriptorSet set = VK_NULL_HANDLE;
		allocateSets({ &layout, 1 }, { &set, 1 });
		return set;
	}

	[[nodiscard]] VkDescriptorSet allocateSet(const usize layout_index = 0) const {
		return allocateSet(layouts[layout_index].handle());
	}

};

void writeDescriptorSetBuffer(VkDescriptorSet set, const uint32_t binding, const VkBuffer buffer, const VkDeviceSize range, const VkDeviceSize offset = 0) noexcept;
void writeDescriptorSetImage(VkDescriptorSet set, const uint32_t binding, const VkSampler sampler, const VkImageView view, const VkImageLayout layout) noexcept;

using ShaderModule = VulkanResource<VkShaderModule, [](VkShaderModule handle) {
	vkDestroyShaderModule(device(), handle, nullptr);
}>;

using PipelineLayout = VulkanResource<VkPipelineLayout, [](VkPipelineLayout handle) {
	vkDestroyPipelineLayout(device(), handle, nullptr);
}>;

struct Pipeline {
	std::vector<ShaderModule> shader_modules;
	PipelineLayout layout;
	VulkanResource <VkPipeline, [](VkPipeline handle) {
		vkDestroyPipeline(device(), handle, nullptr);
	}> pipeline;

	[[nodiscard]] constexpr VkPipeline handle() const {
		return pipeline.handle();
	}
	[[nodiscard]] constexpr VkPipeline* ptr() {
		return pipeline.ptr();
	}

	struct CreateInfo {
		struct RenderTarget {
			//std::vector<VkFormat> color_formats = { VK_FORMAT_UNDEFINED };
			VkFormat color_format = VK_FORMAT_UNDEFINED;
			VkFormat depth_format = VK_FORMAT_UNDEFINED;
			VkFormat stencil_format = VK_FORMAT_UNDEFINED;
		};

		std::string_view vertex_shader;
		std::string_view fragment_shader;
		std::vector<VkVertexInputBindingDescription> binding_descriptions;
		std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
		std::span<const VkDescriptorSetLayout> descriptor_layouts;
		std::optional<VkPushConstantRange> push_constant = {};
		RenderTarget render_target;
		// cull mode
		// multisampling
	};

	[[nodiscard]] static Pipeline createGraphics(const CreateInfo& create_info);
};


struct CommandBuffer {
	VulkanResource<VkCommandBuffer, [](VkCommandBuffer handle) {
		vkFreeCommandBuffers(device(), commandPool(), 1, &handle);
	}> cmd;

	constexpr CommandBuffer() noexcept { }
	constexpr CommandBuffer(VkCommandBuffer handle) noexcept : cmd(handle) { }

	[[nodiscard]] constexpr VkCommandBuffer handle() const {
		return cmd.handle();
	}
	[[nodiscard]] constexpr VkCommandBuffer* ptr() {
		return cmd.ptr();
	}

	static void allocate(const std::span<VkCommandBuffer> dst_buffers, const VkQueueFlags type = VK_QUEUE_GRAPHICS_BIT) {
		assert(type & VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT && "Only graphics and compute command buffers are supported");
		VkCommandBufferAllocateInfo alloc_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = commandPool(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = static_cast<u32>(dst_buffers.size()),
		};
		if (vkAllocateCommandBuffers(device(), &alloc_info, dst_buffers.data()) != VK_SUCCESS || dst_buffers[0] == VK_NULL_HANDLE) {
			throw Err::Vulkan;
		}
	}
	[[nodiscard]] static VkCommandBuffer allocate(const VkQueueFlags type = VK_QUEUE_GRAPHICS_BIT) {
		VkCommandBuffer buffer = VK_NULL_HANDLE;
		allocate({ &buffer, 1 }, type);
		return buffer;
	}

	void begin(VkCommandBufferUsageFlags flags = 0) const {
		VkCommandBufferBeginInfo begin_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = flags,
		};
		vkBeginCommandBuffer(handle(), &begin_info);
	}
	void endAndSubmitSimple(VkFence fence = VK_NULL_HANDLE) {
		if (vkEndCommandBuffer(handle()) != VK_SUCCESS) {
			throw Err::Vulkan;
		}
		VkSubmitInfo submit_info = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = ptr(),
		};
		if (vkQueueSubmit(queue(), 1, &submit_info, fence) != VK_SUCCESS) {
			throw Err::Vulkan;
		}
	}

	void copyBufferToBuffer(const VkBuffer src_buffer, const VkBuffer dst_buffer, const VkDeviceSize size, const VkDeviceSize src_offset = 0, const VkDeviceSize dst_offset = 0) const noexcept {
		VkBufferCopy copy_region = {
			.srcOffset = src_offset,
			.dstOffset = dst_offset,
			.size = size,
		};
		vkCmdCopyBuffer(handle(), src_buffer, dst_buffer, 1, &copy_region);
	}
	void copyBufferToImage(const VkBuffer src_buffer, VkImage dst_image, const VkExtent3D extent) const noexcept {
		VkBufferImageCopy region = {
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = extent,
		};
		vkCmdCopyBufferToImage(handle(), src_buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}
	void insertImageMemoryBarrier(const VkImage image,
		const VkImageLayout old_layout, const VkImageLayout new_layout,
		const VkPipelineStageFlags src_stage, const VkPipelineStageFlags dst_stage,
		const VkAccessFlags src_access_mask, const VkAccessFlags dst_access_mask,
		const VkImageAspectFlags aspect_mask, const u32 mip_level
	) const noexcept {
		VkImageMemoryBarrier barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = src_access_mask,
			.dstAccessMask = dst_access_mask,
			.oldLayout = old_layout,
			.newLayout = new_layout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				.aspectMask = aspect_mask,
				.baseMipLevel = 0,
				.levelCount = mip_level,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		vkCmdPipelineBarrier(handle(), src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void setViewportAndScissor(const Vec2<i32> extent) const noexcept {
		VkViewport viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(extent.x),
			.height = static_cast<float>(extent.y),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};
		vkCmdSetViewport(handle(), 0, 1, &viewport);
		VkRect2D scissor = { 
			.offset = { 0, 0 },
			.extent = { static_cast<u32>(extent.x), static_cast<u32>(extent.y) }
		};
		vkCmdSetScissor(handle(), 0, 1, &scissor);
	}
};

constexpr VkSurfaceFormatKHR SurfaceFormat = {
	.format = VK_FORMAT_B8G8R8A8_SRGB,
	.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
};

using Surface = VulkanResource<VkSurfaceKHR, [](VkSurfaceKHR handle) {
	vkDestroySurfaceKHR(instance(), handle, nullptr);
}>;

[[nodiscard]] inline VkSurfaceKHR createSurface(SDL_Window* window) {
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	if (!SDL_Vulkan_CreateSurface(window, instance(), nullptr, &surface) || surface == VK_NULL_HANDLE) {
		std::println("{}", SDL_GetError());
		throw Err::Vulkan;
	}
	return surface;
}

struct Swapchain {
	static constexpr u32 MaxImages = 3;

	VulkanResource<VkSwapchainKHR, [](VkSwapchainKHR handle) {
		vkDestroySwapchainKHR(device(), handle, nullptr);
	}> swapchain;
	u32 current_image = 0;
	u32 image_count = 0;
	std::array<VkImage, MaxImages> images{};
	std::array<ImageView, MaxImages> image_views{};

	[[nodiscard]] constexpr VkSwapchainKHR handle() const {
		return swapchain.handle();
	}
	[[nodiscard]] constexpr VkSwapchainKHR* ptr() {
		return swapchain.ptr();
	}

	[[nodiscard]] static Swapchain create(const Vec2<i32> window_size, const VkSurfaceKHR surface, const VkSwapchainKHR old_swapchain);
};

}
