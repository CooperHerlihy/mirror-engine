#include "reflect/furnace.h"

#include <fstream>
#include <print>

namespace Mirror::Vk {

#ifndef NDEBUG
static VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info = {
	.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
	.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
					   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
	.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
	.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT,
						  const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void*) -> VkBool32 {
		std::println("validation layer: {}", callback_data->pMessage);
		debug_assert(!(severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT));
		return VK_FALSE;
	},
};
#endif

[[nodiscard]] static bool check_validation_layer_availability() {
	u32 available_layer_count;
	release_assert(vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr) == VK_SUCCESS);
	std::vector<VkLayerProperties> available_layers{available_layer_count};
	release_assert(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data()) == VK_SUCCESS);

	for (const char* required_layer_name : ValidationLayers) {
		bool found = false;
		for (const auto& available_layer : available_layers) {
			found = strcmp(available_layer.layerName, required_layer_name) == 0;
			if (found) {
				break;
			}
		}
		if (!found) {
			return false;
		}
	}
	return true;
}

[[nodiscard]] static std::vector<const char*> get_required_instance_extensions() {
	u32 sdl_extension_count = 0;
	const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count);
	release_assert(sdl_extensions);

	std::vector<const char*> required_extensions;
	required_extensions.reserve(static_cast<usize>(sdl_extension_count) + 1);
	required_extensions.append_range(std::span{sdl_extensions, sdl_extension_count});
#ifndef NDEBUG
	required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
	return required_extensions;
}

[[nodiscard]] static bool check_instance_extension_availability(const std::span<const char*> required_extensions) {
	u32 extension_count = 0;
	release_assert(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr) == VK_SUCCESS);
	std::vector<VkExtensionProperties> extensions{extension_count};
	release_assert(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data()) == VK_SUCCESS);

	for (const char* required : required_extensions) {
		bool found = false;
		for (const auto& extension : extensions) {
			found = strcmp(required, extension.extensionName) == 0;
			if (found) {
				break;
			}
		}
		if (!found) {
			return false;
		}
	}
	return true;
}

[[nodiscard]] static VkInstance create_instance(const std::string_view app_name) {
	std::vector<const char*> required_extensions = get_required_instance_extensions();
	release_assert(check_instance_extension_availability(required_extensions));

	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = app_name.data(),
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Mirror Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_3,
	};
	VkInstanceCreateInfo instance_create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#ifdef NDEBUG
		.pNext = nullptr,
#else
		.pNext = &debug_utils_messenger_create_info,
#endif
		.pApplicationInfo = &app_info,
		.enabledLayerCount = static_cast<u32>(ValidationLayers.size()),
		.ppEnabledLayerNames = ValidationLayers.data(),
		.enabledExtensionCount = static_cast<u32>(required_extensions.size()),
		.ppEnabledExtensionNames = required_extensions.data(),
	};
	VkInstance instance;
	release_assert(vkCreateInstance(&instance_create_info, nullptr, &instance) == VK_SUCCESS);
	release_assert(instance != VK_NULL_HANDLE);
	return instance;
}

#ifndef NDEBUG
[[nodiscard]] static VkDebugUtilsMessengerEXT create_debug_messenger() {
	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_furnace.instance, "vkCreateDebugUtilsMessengerEXT");
	debug_assert(vkCreateDebugUtilsMessengerEXT != nullptr);
	VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
	debug_assert(vkCreateDebugUtilsMessengerEXT(g_furnace.instance, &debug_utils_messenger_create_info, nullptr,
												&messenger) == VK_SUCCESS);
	debug_assert(messenger != VK_NULL_HANDLE);
	return messenger;
}
#endif

[[nodiscard]] static std::vector<VkPhysicalDevice> get_physical_devices() {
	u32 physical_device_count = 0;
	release_assert(vkEnumeratePhysicalDevices(g_furnace.instance, &physical_device_count, nullptr) == VK_SUCCESS);
	std::vector<VkPhysicalDevice> physical_devices{physical_device_count};
	release_assert(vkEnumeratePhysicalDevices(g_furnace.instance, &physical_device_count, physical_devices.data()) ==
				   VK_SUCCESS);
	return physical_devices;
}

[[nodiscard]] static bool check_device_extension_availability(const VkPhysicalDevice physical_device) {
	u32 extension_count = 0;
	release_assert(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr) ==
				   VK_SUCCESS);
	std::vector<VkExtensionProperties> extensions{extension_count};
	release_assert(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count,
														extensions.data()) == VK_SUCCESS);

	for (const char* required : RequiredDeviceExtensions) {
		bool found = false;
		for (const auto& extension : extensions) {
			found = strcmp(required, extension.extensionName) == 0;
			if (found) {
				break;
			}
		}
		if (!found) {
			return false;
		}
	}
	return true;
}

[[nodiscard]] static std::optional<QueueFamily> get_queue_family(const VkPhysicalDevice physical_device,
																 const VkQueueFlags flags) {
	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> queue_families{queue_family_count};
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

	for (u32 family = 0; family < queue_family_count; family++) {
		if (queue_families[family].queueFlags & flags) {
			return std::optional{family};
		}
	}
	return {};
}

struct PhysicalDeviceResult {
	VkPhysicalDevice physical_device;
	QueueFamily queue_family;
};
[[nodiscard]] static std::optional<PhysicalDeviceResult>
find_suitable_physical_device(const std::span<const VkPhysicalDevice> physical_devices) {
	for (VkPhysicalDevice physical_device : physical_devices) {
		VkPhysicalDeviceFeatures supported_features;
		vkGetPhysicalDeviceFeatures(physical_device, &supported_features);
		if (!supported_features.samplerAnisotropy || !supported_features.sampleRateShading) {
			continue;
		}
		if (!check_device_extension_availability(physical_device)) {
			continue;
		}
		auto queue_family = get_queue_family(physical_device, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
		if (!queue_family.has_value()) {
			continue;
		}
		return std::optional{PhysicalDeviceResult{physical_device, queue_family.value()}};
	}
	return {};
}

[[nodiscard]] static VkDevice create_device() {
	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
		.dynamicRendering = VK_TRUE,
	};
	VkPhysicalDeviceFeatures device_features{
		.sampleRateShading = VK_TRUE,
		.samplerAnisotropy = VK_TRUE,
	};
	float queue_priority = 1.0f;
	VkDeviceQueueCreateInfo queue_info{.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
									   .queueFamilyIndex = g_furnace.queue_family,
									   .queueCount = 1,
									   .pQueuePriorities = &queue_priority};
	VkDeviceCreateInfo create_info{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &dynamic_rendering_feature,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queue_info,
#ifdef NDEBUG
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
#else
		.enabledLayerCount = static_cast<u32>(ValidationLayers.size()),
		.ppEnabledLayerNames = ValidationLayers.data(),
#endif
		.enabledExtensionCount = static_cast<u32>(RequiredDeviceExtensions.size()),
		.ppEnabledExtensionNames = RequiredDeviceExtensions.data(),
		.pEnabledFeatures = &device_features,
	};
	VkDevice device = VK_NULL_HANDLE;
	release_assert(vkCreateDevice(g_furnace.physical_device, &create_info, nullptr, &device) == VK_SUCCESS);
	release_assert(device != VK_NULL_HANDLE);
	return device;
}

[[nodiscard]] static VkCommandPool create_command_pool() {
	VkCommandPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = g_furnace.queue_family,
	};
	VkCommandPool cmd_pool = VK_NULL_HANDLE;
	release_assert(vkCreateCommandPool(g_furnace.device, &pool_info, nullptr, &cmd_pool) == VK_SUCCESS);
	release_assert(cmd_pool != VK_NULL_HANDLE);
	return cmd_pool;
}

FurnaceKeeper::FurnaceKeeper(const std::string_view app_name, const std::string_view app_version) : FurnaceKeeper() {
	debug_assert(g_furnace.SDL_initialized == false);
	release_assert(SDL_SetAppMetadata(app_name.data(), app_version.data(), nullptr));
	release_assert(SDL_Init(SDL_INIT_VIDEO));
	g_furnace.SDL_initialized = true;

	debug_assert(g_furnace.instance == VK_NULL_HANDLE);
	debug_assert(check_validation_layer_availability());
	g_furnace.instance = create_instance(app_name);
	debug_assert(g_furnace.instance != VK_NULL_HANDLE);

#ifndef NDEBUG
	debug_assert(g_furnace.debug_messenger == VK_NULL_HANDLE);
	g_furnace.debug_messenger = create_debug_messenger();
	debug_assert(g_furnace.debug_messenger != VK_NULL_HANDLE);
#endif

	debug_assert(g_furnace.physical_device == VK_NULL_HANDLE);
	debug_assert(g_furnace.queue_family == UINT32_MAX);
	std::vector<VkPhysicalDevice> physical_devices = get_physical_devices();
	auto physical_device_result = find_suitable_physical_device(physical_devices);
	release_assert(physical_device_result.has_value());
	g_furnace.physical_device = physical_device_result.value().physical_device;
	g_furnace.queue_family = physical_device_result.value().queue_family;
	debug_assert(g_furnace.physical_device != VK_NULL_HANDLE);
	debug_assert(g_furnace.queue_family != UINT32_MAX);

	debug_assert(g_furnace.device == VK_NULL_HANDLE);
	g_furnace.device = create_device();
	debug_assert(g_furnace.device != VK_NULL_HANDLE);

	debug_assert(g_furnace.queue == VK_NULL_HANDLE);
	vkGetDeviceQueue(g_furnace.device, g_furnace.queue_family, 0, &g_furnace.queue);
	release_assert(g_furnace.queue != VK_NULL_HANDLE)

		debug_assert(g_furnace.command_pool == VK_NULL_HANDLE);
	g_furnace.command_pool = create_command_pool();
	debug_assert(g_furnace.command_pool != VK_NULL_HANDLE);
}

FurnaceKeeper::~FurnaceKeeper() noexcept {
	if (g_furnace.device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(g_furnace.device);
	}
	if (g_furnace.command_pool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(g_furnace.device, g_furnace.command_pool, nullptr);
		g_furnace.command_pool = VK_NULL_HANDLE;
	}
	if (g_furnace.device != VK_NULL_HANDLE) {
		vkDestroyDevice(g_furnace.device, nullptr);
		g_furnace.device = VK_NULL_HANDLE;
	}
	g_furnace.physical_device = VK_NULL_HANDLE;
	g_furnace.queue_family = UINT32_MAX;
#ifndef NDEBUG
	if (g_furnace.debug_messenger != VK_NULL_HANDLE) {
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
			(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_furnace.instance,
																	   "vkDestroyDebugUtilsMessengerEXT");
		vkDestroyDebugUtilsMessengerEXT(g_furnace.instance, g_furnace.debug_messenger, nullptr);
		g_furnace.debug_messenger = VK_NULL_HANDLE;
	}
#endif
	if (g_furnace.instance != VK_NULL_HANDLE) {
		vkDestroyInstance(g_furnace.instance, nullptr);
		g_furnace.instance = VK_NULL_HANDLE;
	}
	if (g_furnace.SDL_initialized) {
		SDL_Quit();
		g_furnace.SDL_initialized = false;
	}
}

std::optional<VkFormat> find_supported_format(const std::span<const VkFormat> candidates, const VkImageTiling tiling,
											  const VkFormatFeatureFlags features) noexcept {
	for (auto candidate : candidates) {
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physical_device(), candidate, &properties);
		if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
			return candidate;
		}
		/*else if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures &
		features) == features) { return candidate
		}*/
	}
	return {};
}

std::optional<u32> find_memory_type_index(const u32 type_filter, const VkMemoryPropertyFlags properties) noexcept {
	VkPhysicalDeviceMemoryProperties mem_props;
	vkGetPhysicalDeviceMemoryProperties(physical_device(), &mem_props);
	for (u32 i = 0; i < mem_props.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return {};
}

VkFormat get_depth_format() {
	const static VkFormat format = [] {
		auto depth_format = Vk::find_supported_format(
			std::array{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		release_assert(depth_format.has_value());
		return depth_format.value();
	}();
	return format;
}

VkSemaphore create_semaphore() {
	constexpr VkSemaphoreCreateInfo semaphore_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	VkSemaphore semaphore = VK_NULL_HANDLE;
	release_assert(vkCreateSemaphore(device(), &semaphore_info, nullptr, &semaphore) == VK_SUCCESS);
	release_assert(semaphore != VK_NULL_HANDLE);
	return semaphore;
}

[[nodiscard]] VkFence create_fence(const VkFenceCreateFlags flags) {
	VkFenceCreateInfo semaphore_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = flags,
	};
	VkFence fence = VK_NULL_HANDLE;
	release_assert(vkCreateFence(device(), &semaphore_info, nullptr, &fence) == VK_SUCCESS);
	release_assert(fence != VK_NULL_HANDLE);
	return fence;
}

VkSampler create_sampler(const VkFilter type) {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physical_device(), &properties);
	VkSamplerCreateInfo sampler_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = type,
		.minFilter = type,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipLodBias = 0.0f,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = 0.0f,
		.maxLod = 0.0f,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
	};
	VkSampler sampler = VK_NULL_HANDLE;
	release_assert(vkCreateSampler(device(), &sampler_info, nullptr, &sampler) == VK_SUCCESS);
	release_assert(sampler != VK_NULL_HANDLE);
	return sampler;
}

VkDeviceMemory create_device_memory(const VkMemoryRequirements requirements, const MemoryType type) {
	auto memory_type_index_result =
		find_memory_type_index(requirements.memoryTypeBits, static_cast<VkMemoryPropertyFlags>(type));
	release_assert(memory_type_index_result.has_value());
	u32 memory_type_index = memory_type_index_result.value();

	VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = requirements.size,
		.memoryTypeIndex = memory_type_index,
	};
	VkDeviceMemory device_memory;
	release_assert(vkAllocateMemory(device(), &alloc_info, nullptr, &device_memory) == VK_SUCCESS);
	release_assert(device_memory != VK_NULL_HANDLE);
	return device_memory;
}

void writeToHostVisibleMemory(const VkDeviceMemory dst, const void* src, const VkDeviceSize size,
							  const VkDeviceSize offset) {
	debug_assert(dst != VK_NULL_HANDLE);
	debug_assert(src != nullptr);
	debug_assert(size != 0);

	void* map;
	release_assert(vkMapMemory(device(), dst, offset, size, 0, &map) == VK_SUCCESS);
	memcpy((u8*)map, src, size);
	vkUnmapMemory(device(), dst);
}

Buffer Buffer::create(const CreateInfo create_info) {
	debug_assert(create_info.size != 0);
	debug_assert(create_info.usage != static_cast<VkBufferUsageFlags>(0));
	debug_assert(create_info.memory_type != static_cast<MemoryType>(0));

	Buffer buffer;

	VkBufferCreateInfo buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = create_info.size,
		.usage = create_info.usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &g_furnace.queue_family,
	};
	if (create_info.memory_type == MemoryType::DeviceLocal) {
		buffer_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}
	release_assert(vkCreateBuffer(device(), &buffer_info, nullptr, buffer.ptr()) == VK_SUCCESS);
	release_assert(buffer.buffer.valid());

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);
	buffer.memory = create_device_memory(mem_reqs, create_info.memory_type);
	debug_assert(buffer.memory.valid());

	release_assert(vkBindBufferMemory(device(), buffer.handle(), buffer.memory.handle(), 0) == VK_SUCCESS);

	debug_assert(buffer.valid());
	return buffer;
}

void write_device_local_buffer(const Buffer& dst_buffer, const void* src_data, const VkDeviceSize size,
							   const VkDeviceSize offset) {
	debug_assert(src_data != nullptr);
	debug_assert(dst_buffer.valid());
	debug_assert(size != 0);

	Buffer staging_buffer = Buffer::create({
		.size = size,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.memory_type = MemoryType::HostVisible,
	});
	debug_assert(staging_buffer.valid());
	writeToHostVisibleMemory(staging_buffer.memory.handle(), src_data, size, offset);

	CommandBuffer cmd = create_command_buffer();
	debug_assert(cmd.valid());

	cmd_begin(cmd.handle(), VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	cmd_copy_buffer_to_buffer(cmd.handle(), staging_buffer.handle(), dst_buffer.handle(), size, offset, offset);

	Fence fence = create_fence();
	debug_assert(fence.valid());
	cmd_end_and_submit_simple(cmd.handle(), fence.handle());
	wait_for_fence(fence.handle());
}

Image Image::create(const CreateInfo& create_info) {
	debug_assert(create_info.extent.height > 0);
	debug_assert(create_info.extent.width > 0);
	debug_assert(create_info.extent.depth > 0);
	debug_assert(create_info.usage != static_cast<VkImageUsageFlags>(0));
	debug_assert(create_info.memory_type != static_cast<MemoryType>(0));
	debug_assert(create_info.format != VK_FORMAT_UNDEFINED);
	debug_assert(create_info.image_type >= VK_IMAGE_TYPE_1D && create_info.image_type <= VK_IMAGE_TYPE_3D);
	debug_assert(create_info.aspect_mask != static_cast<VkImageAspectFlags>(0));
	debug_assert(create_info.tiling == VK_IMAGE_TILING_OPTIMAL || create_info.tiling == VK_IMAGE_TILING_LINEAR);
	debug_assert(create_info.samples != static_cast<VkSampleCountFlagBits>(0));
	debug_assert(create_info.mip_levels != 0);
	debug_assert(create_info.array_layers != 0);

	Image image;

	VkImageCreateInfo image_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = create_info.image_type,
		.format = create_info.format,
		.extent = create_info.extent,
		.mipLevels = create_info.mip_levels,
		.arrayLayers = create_info.array_layers,
		.samples = create_info.samples,
		.tiling = create_info.tiling,
		.usage = create_info.usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &g_furnace.queue_family,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};
	if (create_info.memory_type == MemoryType::DeviceLocal) {
		image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	release_assert(vkCreateImage(device(), &image_info, nullptr, image.ptr()) == VK_SUCCESS);
	release_assert(image.image.valid());

	VkMemoryRequirements mem_reqs;
	vkGetImageMemoryRequirements(device(), image.handle(), &mem_reqs);
	image.memory = create_device_memory(mem_reqs, create_info.memory_type);
	debug_assert(image.memory.valid());

	release_assert(vkBindImageMemory(device(), image.handle(), image.memory.handle(), 0) == VK_SUCCESS);

	VkImageViewCreateInfo view_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image.handle(),
		.viewType = static_cast<VkImageViewType>(create_info.image_type),
		.format = create_info.format,
		.components{
			.r = VK_COMPONENT_SWIZZLE_R,
			.g = VK_COMPONENT_SWIZZLE_G,
			.b = VK_COMPONENT_SWIZZLE_B,
			.a = VK_COMPONENT_SWIZZLE_A,
		},
		.subresourceRange{
			.aspectMask = create_info.aspect_mask,
			.baseMipLevel = 0,
			.levelCount = create_info.mip_levels,
			.baseArrayLayer = 0,
			.layerCount = create_info.array_layers,
		},
	};
	release_assert(vkCreateImageView(device(), &view_info, nullptr, image.view.ptr()) == VK_SUCCESS);
	release_assert(image.view.valid());

	debug_assert(image.valid());
	return image;
}

void write_device_local_image(const Image& dst_image, const void* src_data, const VkExtent3D extent,
							  const u32 pixel_alignment, const VkImageLayout final_layout) {
	debug_assert(src_data != nullptr);
	debug_assert(dst_image.valid());
	debug_assert(extent.height > 0);
	debug_assert(extent.width > 0);
	debug_assert(extent.depth > 0);
	debug_assert(pixel_alignment != 0);

	VkDeviceSize size = (VkDeviceSize)extent.width * (VkDeviceSize)extent.height * (VkDeviceSize)extent.depth *
						(VkDeviceSize)pixel_alignment;
	Buffer staging_buffer = Buffer::create({
		.size = size,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.memory_type = MemoryType::HostVisible,
	});
	debug_assert(staging_buffer.valid());
	writeToHostVisibleMemory(staging_buffer.memory.handle(), src_data, size);

	CommandBuffer cmd = create_command_buffer();
	debug_assert(cmd.valid());

	cmd_begin(cmd.handle(), VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	cmd_insert_image_memory_barrier(cmd.handle(), dst_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED,
									VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
									VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT);
	cmd_copy_buffer_to_image(cmd.handle(), staging_buffer.handle(), dst_image.handle(), extent);
	cmd_insert_image_memory_barrier(cmd.handle(), dst_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
									final_layout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
									VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	Fence fence = create_fence();
	debug_assert(fence.valid());
	cmd_end_and_submit_simple(cmd.handle(), fence.handle());
	wait_for_fence(fence.handle());
}

DescriptorPool DescriptorPool::create(const CreateInfo& create_info) {
	debug_assert(!create_info.sets.empty());

	DescriptorPool descriptor_pool;

	for (const auto& set : create_info.sets) {
		VkDescriptorSetLayoutCreateInfo layout_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<u32>(set.bindings.size()),
			.pBindings = set.bindings.data(),
		};
		VkDescriptorSetLayout layout = VK_NULL_HANDLE;
		release_assert(vkCreateDescriptorSetLayout(device(), &layout_info, nullptr, &layout) == VK_SUCCESS);
		release_assert(layout != VK_NULL_HANDLE);
		descriptor_pool.layouts.push_back(layout);
	}

	u32 max_sets = 0;
	std::vector<VkDescriptorPoolSize> sizes;
	for (const auto& set : create_info.sets) {
		max_sets += set.count;
		for (auto& binding : set.bindings) {
			if (auto it = std::find_if(sizes.begin(), sizes.end(),
									   [=](const VkDescriptorPoolSize size) {
										   return size.type == binding.descriptorType;
									   });
				it != sizes.end()) {
				it->descriptorCount += binding.descriptorCount * set.count;
			} else {
				sizes.emplace_back(static_cast<VkDescriptorType>(binding.descriptorType),
								   binding.descriptorCount * set.count);
			}
		}
	}
	debug_assert(!sizes.empty());

	VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		.maxSets = max_sets,
		.poolSizeCount = static_cast<u32>(sizes.size()),
		.pPoolSizes = sizes.data(),
	};
	release_assert(vkCreateDescriptorPool(device(), &pool_info, nullptr, descriptor_pool.ptr()) == VK_SUCCESS);
	release_assert(descriptor_pool.pool.valid());

	debug_assert(descriptor_pool.valid());
	return descriptor_pool;
}

void DescriptorPool::allocate_sets(const std::span<const VkDescriptorSetLayout> set_layouts,
								   const std::span<VkDescriptorSet> out_sets) const {
	debug_assert(set_layouts.size() == out_sets.size());

	VkDescriptorSetAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = handle(),
		.descriptorSetCount = static_cast<u32>(set_layouts.size()),
		.pSetLayouts = set_layouts.data(),
	};
	release_assert(vkAllocateDescriptorSets(device(), &alloc_info, out_sets.data()) == VK_SUCCESS);
	for (const auto& set : out_sets) {
		release_assert(set != VK_NULL_HANDLE);
	}
}

void write_descriptor_set_buffer(const VkDescriptorSet set, const uint32_t binding, const VkBuffer buffer,
								 const VkDeviceSize range, const VkDeviceSize offset) noexcept {
	debug_assert(set != VK_NULL_HANDLE);
	debug_assert(buffer != VK_NULL_HANDLE);
	debug_assert(range != 0);

	VkDescriptorBufferInfo buffer_info{buffer, offset, range};
	VkWriteDescriptorSet descriptor_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = set,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pBufferInfo = &buffer_info,
	};
	vkUpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
}

void write_descriptor_set_image(const VkDescriptorSet set, const uint32_t binding, const VkSampler sampler,
								const VkImageView view, const VkImageLayout layout) noexcept {
	debug_assert(set != VK_NULL_HANDLE);
	debug_assert(sampler != VK_NULL_HANDLE);
	debug_assert(view != VK_NULL_HANDLE);
	debug_assert(layout != VK_IMAGE_LAYOUT_UNDEFINED);

	VkDescriptorImageInfo image_info{sampler, view, layout};
	VkWriteDescriptorSet descriptor_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = set,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = &image_info,
	};
	vkUpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
}

VkShaderModule create_shader_module(const std::string_view path) {
	debug_assert(!path.empty());

	auto file = std::ifstream{path.data(), std::ios::ate | std::ios::binary};
	release_assert(file.is_open());
	usize code_size = (usize)file.tellg();
	std::vector<char> code;
	code.resize(code_size);
	file.seekg(0);
	file.read(code.data(), code.size());
	file.close();

	VkShaderModuleCreateInfo shader_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size(),
		.pCode = reinterpret_cast<u32*>(code.data()),
	};
	VkShaderModule shader;
	release_assert(vkCreateShaderModule(device(), &shader_info, nullptr, &shader) == VK_SUCCESS);
	release_assert(shader != VK_NULL_HANDLE);
	return shader;
}

[[nodiscard]] GraphicsPipeline GraphicsPipeline::create_graphics(const CreateInfo& create_info) {
	debug_assert(!create_info.vertex_shader.empty());
	debug_assert(!create_info.fragment_shader.empty());
	debug_assert(create_info.render_target.color_format != VK_FORMAT_UNDEFINED ||
				 create_info.render_target.depth_format != VK_FORMAT_UNDEFINED ||
				 create_info.render_target.stencil_format != VK_FORMAT_UNDEFINED)
	debug_assert(create_info.MSAA != static_cast<VkSampleCountFlagBits>(0));
	debug_assert(create_info.cull_mode >= VK_CULL_MODE_NONE && create_info.cull_mode <= VK_CULL_MODE_FRONT_AND_BACK);

	GraphicsPipeline pipeline;

	pipeline.vertex_shader = create_shader_module(create_info.vertex_shader);
	debug_assert(pipeline.vertex_shader.valid());
	pipeline.fragment_shader = create_shader_module(create_info.fragment_shader);
	debug_assert(pipeline.fragment_shader.valid());

	VkPipelineLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = static_cast<u32>(create_info.descriptor_layouts.size()),
		.pSetLayouts = create_info.descriptor_layouts.data(),
		.pushConstantRangeCount = create_info.push_constant.has_value() ? 1U : 0U,
		.pPushConstantRanges = create_info.push_constant.has_value() ? &create_info.push_constant.value() : nullptr,
	};
	release_assert(vkCreatePipelineLayout(device(), &layout_info, nullptr, pipeline.layout.ptr()) == VK_SUCCESS);
	release_assert(pipeline.layout.valid());

	VkPipelineRenderingCreateInfo dynamic_rendering_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
		//.colorAttachmentCount = static_cast<u32>(create_info.render_target.color_formats.size()),
		//.pColorAttachmentFormats = &create_info.render_target.color_formats.data(),
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &create_info.render_target.color_format,
		.depthAttachmentFormat = create_info.render_target.depth_format,
		.stencilAttachmentFormat = create_info.render_target.stencil_format,
	};
	VkPipelineShaderStageCreateInfo shader_stage_infos[] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = pipeline.vertex_shader.handle(),
			.pName = "main",
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = pipeline.fragment_shader.handle(),
			.pName = "main",
		},
	};
	VkPipelineVertexInputStateCreateInfo vertex_input_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = static_cast<u32>(create_info.binding_descriptions.size()),
		.pVertexBindingDescriptions = create_info.binding_descriptions.data(),
		.vertexAttributeDescriptionCount = static_cast<u32>(create_info.attribute_descriptions.size()),
		.pVertexAttributeDescriptions = create_info.attribute_descriptions.data(),
	};
	VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};
	VkPipelineTessellationStateCreateInfo tessellation_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		.patchControlPoints = 0,
	};
	VkPipelineViewportStateCreateInfo viewport_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = nullptr,
		.scissorCount = 1,
		.pScissors = nullptr,
	};
	VkPipelineRasterizationStateCreateInfo rasterization_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = create_info.cull_mode,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f,
	};
	VkPipelineMultisampleStateCreateInfo multisample_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = create_info.MSAA,
		.sampleShadingEnable = VK_TRUE,
		.minSampleShading = 0.2f,
		.pSampleMask = nullptr,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE,
	};
	VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front = {}, // fix
		.back = {},	 // fix
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
	};
	VkPipelineColorBlendAttachmentState color_blend_attachment = {
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};
	VkPipelineColorBlendStateCreateInfo color_blend_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment,
		.blendConstants = {1.0f, 1.0f, 1.0f, 1.0f},
	};
	std::array dynamic_state_enables{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamic_state_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamic_state_enables.size()),
		.pDynamicStates = dynamic_state_enables.data(),
	};

	VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &dynamic_rendering_info,
		.stageCount = 2,
		.pStages = shader_stage_infos,
		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &input_assembly_info,
		.pTessellationState = &tessellation_info,
		.pViewportState = &viewport_info,
		.pRasterizationState = &rasterization_info,
		.pMultisampleState = &multisample_info,
		.pDepthStencilState = &depth_stencil_info,
		.pColorBlendState = &color_blend_info,
		.pDynamicState = &dynamic_state_info,
		.layout = pipeline.layout.handle(),
		.renderPass = nullptr,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};
	release_assert(vkCreateGraphicsPipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, pipeline.ptr()) ==
				   VK_SUCCESS);
	release_assert(pipeline.pipeline.valid());

	debug_assert(pipeline.valid());
	return pipeline;
}

void allocate_command_buffers(const std::span<VkCommandBuffer> dst_buffers) {
	debug_assert(!dst_buffers.empty());

	VkCommandBufferAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = command_pool(),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<u32>(dst_buffers.size()),
	};
	release_assert(vkAllocateCommandBuffers(device(), &alloc_info, dst_buffers.data()) == VK_SUCCESS);
	for (const auto& cmd_buffer : dst_buffers) {
		release_assert(cmd_buffer != VK_NULL_HANDLE);
	}
}

void cmd_begin(const VkCommandBuffer cmd, const VkCommandBufferUsageFlags flags) {
	debug_assert(cmd != VK_NULL_HANDLE);

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = flags,
	};
	vkBeginCommandBuffer(cmd, &begin_info);
}

void cmd_end_and_submit_simple(const VkCommandBuffer cmd, const VkFence fence) {
	debug_assert(cmd != VK_NULL_HANDLE);

	release_assert(vkEndCommandBuffer(cmd) == VK_SUCCESS);
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd,
	};
	release_assert(vkQueueSubmit(queue(), 1, &submit_info, fence) == VK_SUCCESS);
}

void cmd_set_viewport_and_scissor(const VkCommandBuffer cmd, const VkExtent3D extent) noexcept {
	debug_assert(cmd != VK_NULL_HANDLE);
	debug_assert(extent.height > 0);
	debug_assert(extent.width > 0);
	debug_assert(extent.depth == 1);

	VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(extent.width),
		.height = static_cast<float>(extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = {extent.width, extent.height},
	};
	vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void cmd_copy_buffer_to_buffer(const VkCommandBuffer cmd, const VkBuffer src_buffer, const VkBuffer dst_buffer,
							   const VkDeviceSize size, const VkDeviceSize src_offset,
							   const VkDeviceSize dst_offset) noexcept {
	debug_assert(cmd != VK_NULL_HANDLE);
	debug_assert(src_buffer != VK_NULL_HANDLE);
	debug_assert(dst_buffer != VK_NULL_HANDLE);
	debug_assert(size != 0);

	VkBufferCopy copy_region = {
		.srcOffset = src_offset,
		.dstOffset = dst_offset,
		.size = size,
	};
	vkCmdCopyBuffer(cmd, src_buffer, dst_buffer, 1, &copy_region);
}

void cmd_copy_buffer_to_image(const VkCommandBuffer cmd, const VkBuffer src_buffer, VkImage dst_image,
							  const VkExtent3D extent) noexcept {
	debug_assert(cmd != VK_NULL_HANDLE);
	debug_assert(src_buffer != VK_NULL_HANDLE);
	debug_assert(dst_image != VK_NULL_HANDLE);
	debug_assert(extent.height > 0);
	debug_assert(extent.width > 0);
	debug_assert(extent.depth > 0);

	VkBufferImageCopy region = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = {0, 0, 0},
		.imageExtent = extent,
	};
	vkCmdCopyBufferToImage(cmd, src_buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void cmd_insert_image_memory_barrier(const VkCommandBuffer cmd, const VkImage image, const VkImageLayout old_layout,
									 const VkImageLayout new_layout, const VkPipelineStageFlags src_stage,
									 const VkPipelineStageFlags dst_stage, const VkAccessFlags src_access_mask,
									 const VkAccessFlags dst_access_mask, const VkImageAspectFlags aspect_mask,
									 const u32 mip_level) noexcept {
	debug_assert(cmd != VK_NULL_HANDLE);
	debug_assert(image != VK_NULL_HANDLE);
	debug_assert(mip_level != 0);

	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = src_access_mask,
		.dstAccessMask = dst_access_mask,
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange{
			.aspectMask = aspect_mask,
			.baseMipLevel = 0,
			.levelCount = mip_level,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

VkSurfaceKHR create_surface(SDL_Window* window) {
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	release_assert(SDL_Vulkan_CreateSurface(window, instance(), nullptr, &surface));
	release_assert(surface != VK_NULL_HANDLE);
	return surface;
}

Swapchain Swapchain::create(const Vec2<i32> window_size, const VkSurfaceKHR surface,
							const VkSwapchainKHR old_swapchain) {
	debug_assert(window_size.x > 0);
	debug_assert(window_size.y > 0);
	debug_assert(surface != VK_NULL_HANDLE);

	Swapchain swapchain;

	VkSurfaceCapabilitiesKHR surface_capabilities;
	release_assert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device(), surface, &surface_capabilities) ==
				   VK_SUCCESS);
	Vec2<u32> extent = {static_cast<u32>(window_size.x), static_cast<u32>(window_size.y)};
	if (surface_capabilities.currentExtent.width != UINT32_MAX) {
		extent.x = surface_capabilities.currentExtent.width;
		extent.y = surface_capabilities.currentExtent.height;
	} else {
		extent.x = std::max(surface_capabilities.minImageExtent.width,
							std::min(surface_capabilities.maxImageExtent.width, extent.x));
		extent.y = std::max(surface_capabilities.minImageExtent.height,
							std::min(surface_capabilities.maxImageExtent.height, extent.y));
	}
	release_assert(extent.x > 0);
	release_assert(extent.y > 0);

	u32 img_count = surface_capabilities.minImageCount + 1;
	if (surface_capabilities.maxImageCount > 0 && img_count > surface_capabilities.maxImageCount) {
		img_count = surface_capabilities.maxImageCount;
	}
	release_assert(img_count > 0);

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	uint32_t present_mode_count;
	release_assert(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device(), surface, &present_mode_count,
															 nullptr) == VK_SUCCESS);
	std::vector<VkPresentModeKHR> available_present_modes{present_mode_count};
	release_assert(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device(), surface, &present_mode_count,
															 available_present_modes.data()) == VK_SUCCESS);
	for (auto available_present_mode : available_present_modes) {
		if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			present_mode = available_present_mode;
			break;
		}
	}

	VkSwapchainCreateInfoKHR create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = img_count,
		.imageFormat = SurfaceFormat.format,
		.imageColorSpace = SurfaceFormat.colorSpace,
		.imageExtent = {extent.x, extent.y},
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &g_furnace.queue_family,
		.preTransform = surface_capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.clipped = VK_TRUE,
		.oldSwapchain = old_swapchain,
	};
	release_assert(vkCreateSwapchainKHR(device(), &create_info, nullptr, swapchain.ptr()) == VK_SUCCESS);
	release_assert(swapchain.swapchain.valid());

	release_assert(vkGetSwapchainImagesKHR(device(), swapchain.handle(), &swapchain.image_count, nullptr) ==
				   VK_SUCCESS);
	release_assert(swapchain.image_count > 0);
	release_assert(vkGetSwapchainImagesKHR(device(), swapchain.handle(), &swapchain.image_count,
										   swapchain.images.data()) == VK_SUCCESS);
	for (const auto& image : std::span{swapchain.images.data(), swapchain.image_count}) {
		release_assert(image != VK_NULL_HANDLE);
	}

	for (usize i = 0; i < swapchain.image_count; i++) {
		VkImageViewCreateInfo view_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapchain.images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = SurfaceFormat.format,
			.components{
				.r = VK_COMPONENT_SWIZZLE_R,
				.g = VK_COMPONENT_SWIZZLE_G,
				.b = VK_COMPONENT_SWIZZLE_B,
				.a = VK_COMPONENT_SWIZZLE_A,
			},
			.subresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		release_assert(vkCreateImageView(device(), &view_info, nullptr, swapchain.image_views[i].ptr()) == VK_SUCCESS);
		release_assert(swapchain.image_views[i].valid());
	}

	debug_assert(swapchain.valid());
	return swapchain;
}

} // namespace Mirror::Vk
