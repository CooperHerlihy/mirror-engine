#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "frame.h"

namespace Mirror::Vk {

template<typename T, void (*destructor)(T)>
class VulkanResource {
public:
	[[nodiscard]] T handle() {
		return handle_;
	}

	[[nodiscard]] T* ptr() {
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
	constexpr VulkanResource(VulkanResource&& other) noexcept : handle_(other.handle) {
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

	[[nodiscard]] static constexpr VkInstance instance() {
		return get().instance;
	}
	[[nodiscard]] static constexpr VkDebugUtilsMessengerEXT debugMessenger() {
		return get().debug_messenger;
	}
	[[nodiscard]] static constexpr VkPhysicalDevice physicalDevice() {
		return get().physical_device;
	}
	[[nodiscard]] static constexpr VkDevice device() {
		return get().device;
	}
	[[nodiscard]] static constexpr VkCommandPool commandPool() {
		return get().command_pool;
	}
	[[nodiscard]] static constexpr VkQueue queue() {
		return get().queue;
	}
	[[nodiscard]] static constexpr Furnace::QueueFamily queueFamily() {
		return get().queue_family;
	}
};

[[nodiscard]] std::optional<VkFormat> find_supported_format(std::span<VkFormat> candidates, const VkImageTiling tiling, const VkFormatFeatureFlags features) noexcept;
[[nodiscard]] std::optional<u32> find_memory_type_index(const u32 type_filter, const VkMemoryPropertyFlags properties) noexcept;
[[nodiscard]] VkFormat get_depth_format();

using Semaphore = VulkanResource <VkSemaphore, [](VkSemaphore handle) {
	vkDestroySemaphore(FurnaceKeeper::device(), handle, nullptr);
}>;

[[nodiscard]] inline VkSemaphore createSemaphore() {
	constexpr VkSemaphoreCreateInfo semaphore_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	VkSemaphore semaphore = VK_NULL_HANDLE;
	if (vkCreateSemaphore(FurnaceKeeper::device(), &semaphore_info, nullptr, &semaphore) != VK_SUCCESS || semaphore == VK_NULL_HANDLE) {
		throw Err::Vulkan;
	}
	return semaphore;
}

using Fence = VulkanResource<VkFence, [](VkFence handle) {
	if (handle != VK_NULL_HANDLE) {
		vkDestroyFence(FurnaceKeeper::device(), handle, nullptr);
	}
}>;

[[nodiscard]] inline VkFence createFence() {
	constexpr VkFenceCreateInfo semaphore_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};
	VkFence fence = VK_NULL_HANDLE;
	if (vkCreateFence(FurnaceKeeper::device(), &semaphore_info, nullptr, &fence) != VK_SUCCESS || fence == VK_NULL_HANDLE) {
		throw Err::Vulkan;
	}
	return fence;
}

inline void waitForFences(const std::span<Fence> fences) {
	if (vkWaitForFences(Vk::FurnaceKeeper::device(), static_cast<u32>(fences.size()), fences.data()->ptr(), VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
		throw Err::Vulkan;
	}
}

inline void waitForFence(Fence& fence) {
	waitForFences({ &fence, 1 });
}

using Sampler = VulkanResource<VkSampler, [](VkSampler handle) {
	vkDestroySampler(FurnaceKeeper::device(), handle, nullptr);
}>;

[[nodiscard]] VkSampler createSampler(VkFilter type);

enum struct MemoryType {
	HostVisible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	DeviceLocal = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
};

struct Buffer {
	VkBuffer handle = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;

	constexpr Buffer() noexcept {}
	~Buffer() noexcept {
		if (handle != VK_NULL_HANDLE) {
			vkFreeMemory(Vk::FurnaceKeeper::device(), memory, nullptr);
		}
		if (memory != VK_NULL_HANDLE) {
			vkDestroyBuffer(Vk::FurnaceKeeper::device(), handle, nullptr);
		}
	}
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	constexpr Buffer(Buffer&& other) noexcept : handle(other.handle), memory(other.memory) {
		other.handle = VK_NULL_HANDLE;
		other.memory = VK_NULL_HANDLE;
	}
	constexpr Buffer& operator=(Buffer&& other) noexcept {
		if (this == &other) return *this;
		this->~Buffer();
		handle = other.handle;
		memory = other.memory;
		other.handle = VK_NULL_HANDLE;
		other.memory = VK_NULL_HANDLE;
		return *this;
	}

	struct CreateInfo {
		VkDeviceSize size;
		VkBufferUsageFlags usage;
		MemoryType memory_type;
	};

	[[nodiscard]] static Buffer create(const CreateInfo& create_info);

	void writeHostVisible(const void* data, const VkDeviceSize size, const VkDeviceSize offset = 0) const;
	void writeDeviceLocal(const void* data, const VkDeviceSize size, const VkDeviceSize offset = 0);
};

struct Image {
	VkImage handle = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;

	constexpr Image() noexcept {}
	~Image() noexcept {
		if (view != VK_NULL_HANDLE) {
			vkDestroyImageView(FurnaceKeeper::device(), view, nullptr);
		}
		if (memory != VK_NULL_HANDLE) {
			vkFreeMemory(FurnaceKeeper::device(), memory, nullptr);
		}
		if (handle != VK_NULL_HANDLE) {
			vkDestroyImage(FurnaceKeeper::device(), handle, nullptr);
		}
	}
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;
	constexpr Image(Image&& other) noexcept : handle(other.handle), memory(other.memory), view(other.view) {
		other.handle = VK_NULL_HANDLE;
		other.memory = VK_NULL_HANDLE;
		other.view = VK_NULL_HANDLE;
	}
	constexpr Image& operator=(Image&& other) noexcept {
		if (this == &other) return *this;
		this->~Image();
		handle = other.handle;
		memory = other.memory;
		view = other.view;
		other.handle = VK_NULL_HANDLE;
		other.memory = VK_NULL_HANDLE;
		other.view = VK_NULL_HANDLE;
		return *this;
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
	void writeDeviceLocal(const void* data, const VkExtent3D extent, const u32 pixel_alignment, const VkImageLayout final_layout);
};

struct DescriptorPool {
	VkDescriptorPool handle = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayout> layouts;

	DescriptorPool() noexcept {};
	~DescriptorPool() noexcept {
		for (const auto layout : layouts) {
			if (layout != VK_NULL_HANDLE) {
				vkDestroyDescriptorSetLayout(FurnaceKeeper::device(), layout, nullptr);
			}
		}
		if (handle != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(FurnaceKeeper::device(), handle, nullptr);
		}
	}
	DescriptorPool(const DescriptorPool&) = delete;
	DescriptorPool& operator=(const DescriptorPool&) = delete;
	DescriptorPool(DescriptorPool&& other) noexcept : handle(other.handle) {
		other.handle = VK_NULL_HANDLE;
	}
	DescriptorPool& operator=(DescriptorPool&& other) noexcept {
		if (this == &other) return *this;
		this->~DescriptorPool();
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
		return *this;
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

	[[nodiscard]] VkDescriptorSet allocateSet(const usize layout_index) const {
		return allocateSet(layouts[layout_index]);
	}

};

void writeDescriptorSetBuffer(VkDescriptorSet set, const uint32_t binding, const VkBuffer buffer, const VkDeviceSize range, const VkDeviceSize offset = 0) noexcept;
void writeDescriptorSetImage(VkDescriptorSet set, const uint32_t binding, const VkSampler sampler, const VkImageView view, const VkImageLayout layout) noexcept;

struct Pipeline {
	std::vector<VkShaderModule> shader_modules;
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkPipeline handle = VK_NULL_HANDLE;

	constexpr Pipeline() noexcept { };
	~Pipeline() noexcept {
		if (handle != VK_NULL_HANDLE) {
			vkDestroyPipeline(Vk::FurnaceKeeper::device(), handle, nullptr);
		}
		if (layout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(Vk::FurnaceKeeper::device(), layout, nullptr);
		}
		for (auto shader_module : shader_modules) {
			if (shader_module != VK_NULL_HANDLE) {
				vkDestroyShaderModule(Vk::FurnaceKeeper::device(), shader_module, nullptr);
			}
		}
	};
	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;
	constexpr Pipeline(Pipeline&& other) noexcept
		: shader_modules(std::move(other.shader_modules)), layout(other.layout), handle(other.handle) {
		other.layout = VK_NULL_HANDLE;
		other.handle = VK_NULL_HANDLE;
	}
	constexpr Pipeline& operator=(Pipeline&& other) noexcept {
		if (this == &other) return *this;
		this->~Pipeline();
		shader_modules = std::move(other.shader_modules);
		layout = other.layout;
		handle = other.handle;
		other.layout = VK_NULL_HANDLE;
		other.handle = VK_NULL_HANDLE;
		return *this;
	}

	[[nodiscard]] constexpr operator VkPipeline() const noexcept {
		return handle;
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
	VkCommandBuffer handle = VK_NULL_HANDLE;

	constexpr CommandBuffer() noexcept {}
	constexpr CommandBuffer(const VkCommandBuffer handle) noexcept : handle(handle) {}
	~CommandBuffer() noexcept {
		if (handle != VK_NULL_HANDLE) vkFreeCommandBuffers(FurnaceKeeper::device(), FurnaceKeeper::commandPool(), 1, &handle);
	}

	CommandBuffer(const CommandBuffer&) = delete;
	CommandBuffer& operator=(const CommandBuffer&) = delete;
	CommandBuffer(CommandBuffer&& other) noexcept : handle(other.handle) {
		other.handle = VK_NULL_HANDLE;
	}
	CommandBuffer& operator=(CommandBuffer&& other) noexcept {
		if (this == &other) return *this;
		this->~CommandBuffer();
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
		return *this;
	}

	static void allocate(const VkQueueFlags type, const std::span<VkCommandBuffer> dst_buffers) {
		assert(type & VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT && "Only graphics and compute command buffers are supported");
		VkCommandBufferAllocateInfo alloc_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = FurnaceKeeper::commandPool(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = static_cast<u32>(dst_buffers.size()),
		};
		if (vkAllocateCommandBuffers(FurnaceKeeper::device(), &alloc_info, dst_buffers.data()) != VK_SUCCESS || dst_buffers[0] == VK_NULL_HANDLE) {
			throw Err::Vulkan;
		}
	}
	[[nodiscard]] static VkCommandBuffer allocate(const VkQueueFlags type) {
		VkCommandBuffer buffer = VK_NULL_HANDLE;
		allocate(type, { &buffer, 1 });
		return buffer;
	}

	void begin(VkCommandBufferUsageFlags flags = 0) const {
		VkCommandBufferBeginInfo begin_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = flags,
		};
		vkBeginCommandBuffer(handle, &begin_info);
	}
	void endAndSubmitSimple() const {
		if (vkEndCommandBuffer(handle) != VK_SUCCESS) {
			throw Err::Vulkan;
		}
		VkSubmitInfo submit_info = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &handle,
		};
		if (vkQueueSubmit(FurnaceKeeper::queue(), 1, &submit_info, nullptr) != VK_SUCCESS) {
			throw Err::Vulkan;
		}
	}

	void copyBufferToBuffer(const VkBuffer src_buffer, const VkBuffer dst_buffer, const VkDeviceSize size, const VkDeviceSize src_offset = 0, const VkDeviceSize dst_offset = 0) const noexcept {
		VkBufferCopy copy_region = {
			.srcOffset = src_offset,
			.dstOffset = dst_offset,
			.size = size,
		};
		vkCmdCopyBuffer(handle, src_buffer, dst_buffer, 1, &copy_region);
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
		vkCmdCopyBufferToImage(handle, src_buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
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
		vkCmdPipelineBarrier(handle, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
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
		vkCmdSetViewport(handle, 0, 1, &viewport);
		VkRect2D scissor = { 
			.offset = { 0, 0 },
			.extent = { static_cast<u32>(extent.x), static_cast<u32>(extent.y) }
		};
		vkCmdSetScissor(handle, 0, 1, &scissor);
	}
};

struct Surface {
	static constexpr VkSurfaceFormatKHR Format = {
		.format = VK_FORMAT_B8G8R8A8_SRGB,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	};

	VkSurfaceKHR handle = VK_NULL_HANDLE;

	constexpr Surface() noexcept { }
	constexpr Surface(const VkSurfaceKHR surface) noexcept : handle(surface) { }
	~Surface() noexcept {
		if (handle != VK_NULL_HANDLE) {
			vkDestroySurfaceKHR(FurnaceKeeper::instance(), handle, nullptr);
		}
	}
	Surface(const Surface&) = delete;
	Surface& operator=(const Surface&) = delete;
	constexpr Surface(Surface&& other) noexcept : handle(other.handle) {
		other.handle = VK_NULL_HANDLE;
	}
	Surface& operator=(Surface&& other) noexcept {
		if (this == &other) return *this;
		this->~Surface();
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
		return *this;
	}

	[[nodiscard]] static VkSurfaceKHR create(SDL_Window* window) {
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		if (!SDL_Vulkan_CreateSurface(window, FurnaceKeeper::instance(), nullptr, &surface) || surface == VK_NULL_HANDLE) {
			std::println("{}", SDL_GetError());
			throw Err::Vulkan;
		}
		return surface;
	}
};

struct Swapchain {
	static constexpr u32 MaxImages = 3;

	VkSwapchainKHR handle = VK_NULL_HANDLE;
	u32 current_image = 0;
	u32 image_count = 0;
	std::array<VkImage, MaxImages> images{};
	std::array<VkImageView, MaxImages> image_views{};

	constexpr Swapchain() noexcept { };
	constexpr Swapchain(const VkSwapchainKHR swapchain, const u32 current_image, const u32 image_count, const std::array<VkImage, MaxImages> images, const std::array<VkImageView, MaxImages> image_views) noexcept :
	handle(swapchain), current_image(current_image), image_count(image_count), images(images), image_views(image_views) { };
	~Swapchain() noexcept {
		for (const auto view : image_views) {
			if (view != VK_NULL_HANDLE) {
				vkDestroyImageView(FurnaceKeeper::device(), view, nullptr);
			}
		}
		if (handle != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(FurnaceKeeper::device(), handle, nullptr);
		}
	}
	Swapchain(const Swapchain&) = delete;
	Swapchain& operator=(const Swapchain&) = delete;
	Swapchain(Swapchain&& other) noexcept : handle(other.handle), current_image(other.current_image), image_count(other.image_count), images(other.images), image_views(other.image_views) {
		other.handle = VK_NULL_HANDLE;
		other.images.fill(VK_NULL_HANDLE);
		other.image_views.fill(VK_NULL_HANDLE);
	}
	Swapchain& operator=(Swapchain&& other) noexcept {
		if (this == &other) return *this;
		this->~Swapchain();
		handle = other.handle;
		images = other.images;
		image_views = other.image_views;
		other.handle = VK_NULL_HANDLE;
		other.images.fill(VK_NULL_HANDLE);
		other.image_views.fill(VK_NULL_HANDLE);
		return *this;
	}

	constexpr operator VkSwapchainKHR() {
		return handle;
	}

	[[nodiscard]] static Swapchain create(const Vec2<i32> window_size, const Surface& surface, const VkSwapchainKHR old_swapchain);
};

}

namespace Mirror::Reflect {

void allocateCommandBuffers(const VkQueueFlags type, std::span<VkCommandBuffer> command_buffers);
[[nodiscard]] VkCommandBuffer beginOneTimeCommand();
void endAndSubmitOneTimeCommand(VkCommandBuffer command_buffer);
void cmdCopyBufferToBuffer(const VkCommandBuffer cmd_buf, const Vk::Buffer& src_buffer, const VkDeviceSize src_offset, const VkDeviceSize size, Vk::Buffer& dst_buffer, const VkDeviceSize dst_offset) noexcept;
void cmdCopyBufferToImage(const VkCommandBuffer command_buffer, const Vk::Buffer& src_buffer, const VkExtent3D extent, Vk::Image& dst_image) noexcept;
void cmdInsertImageMemoryBarrier(const VkCommandBuffer command_buffer, const VkImage image,
	const VkImageLayout old_layout, const VkImageLayout new_layout,
	const VkPipelineStageFlags source_stage, const VkPipelineStageFlags dst_stage,
	const VkAccessFlags src_access_mask, const VkAccessFlags dst_access_mask,
	const VkImageAspectFlags aspect_mask, const u32 mip_level
) noexcept;

}
