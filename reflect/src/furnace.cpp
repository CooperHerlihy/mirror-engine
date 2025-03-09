#include "furnace.h"

#include <print>
#include <fstream>

namespace Mirror::Vk {

#ifndef NDEBUG
static VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info = {
	.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
	.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
	.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
	.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void*) -> VkBool32 {
		std::println("validation layer: {}", callback_data->pMessage);
		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			__debugbreak();
			//assert(!"Vulkan Error");
		};
		return VK_FALSE;
	},
};
#endif

FurnaceKeeper::FurnaceKeeper(const std::string_view app_name, const std::string_view app_version) : FurnaceKeeper() {
	Furnace& furnace = get();
	assert(furnace.SDL_initialized == false);
	{
		if (!SDL_SetAppMetadata(app_name.data(), app_version.data(), nullptr)) {
			throw Err::SDL;
		}
		if (!SDL_Init(SDL_INIT_VIDEO)) {
			throw Err::SDL;
		}
		furnace.SDL_initialized = true;
	}
	assert(furnace.instance == VK_NULL_HANDLE);
	{
#ifndef NDEBUG
		{
			u32 available_layer_count;
			if (vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr) != VK_SUCCESS) {
				throw Err::Vulkan;
			}
			std::vector<VkLayerProperties> available_layers{ available_layer_count };
			if (vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data()) != VK_SUCCESS) {
				throw Err::Vulkan;
			}
			bool all_found = true;
			for (const char* required_layer_name : Furnace::ValidationLayers) {
				bool found = false;
				for (const auto& available_layer : available_layers) {
					found = strcmp(available_layer.layerName, required_layer_name) == 0;
					if (found) {
						break;
					}
				}
				if (!found) {
					all_found = false;
					break;
				}
			}
			if (!all_found) {
				throw Err::Vulkan;
			}
		}
#endif
		std::vector<const char*> required_instance_extensions;
		{
			u32 sdl_instance_count = 0;
			const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_instance_count);
			if (!sdl_extensions) {
				throw Err::SDL;
			}
			required_instance_extensions.reserve(static_cast<usize>(sdl_instance_count) + 1);
			required_instance_extensions.append_range(std::span{ sdl_extensions, sdl_instance_count });
#ifndef NDEBUG
			required_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
			u32 instance_extension_count = 0;
			if (vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr) != VK_SUCCESS) {
				throw Err::Vulkan;
			}
			std::vector<VkExtensionProperties> instance_extensions{ instance_extension_count };
			if (vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data()) != VK_SUCCESS) {
				throw Err::Vulkan;
			}
			bool found_instance_extensions = true;
			for (const char* required : required_instance_extensions) {
				bool found_extension = false;
				for (const auto& extension : instance_extensions) {
					found_extension = strcmp(required, extension.extensionName) == 0;
					if (found_extension) {
						break;
					}
				}
				if (!found_extension) {
					found_instance_extensions = false;
					break;
				}
			}
			if (!found_instance_extensions) {
				throw Err::NoResult;
			}
		}

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
			.enabledLayerCount = static_cast<u32>(Furnace::ValidationLayers.size()),
			.ppEnabledLayerNames = Furnace::ValidationLayers.data(),
			.enabledExtensionCount = static_cast<u32>(required_instance_extensions.size()),
			.ppEnabledExtensionNames = required_instance_extensions.data(),
		};
		VkInstance instance = VK_NULL_HANDLE; 
		if (vkCreateInstance(&instance_create_info, nullptr, &instance) != VK_SUCCESS || instance == VK_NULL_HANDLE) {
			throw Err::Vulkan;
		}
		furnace.instance = instance;
	}
#ifndef NDEBUG
	assert(furnace.debug_messenger == VK_NULL_HANDLE);
	{
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(furnace.instance, "vkCreateDebugUtilsMessengerEXT");
		if (!vkCreateDebugUtilsMessengerEXT) throw Err::Vulkan;
		VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
		if (vkCreateDebugUtilsMessengerEXT(furnace.instance, &debug_utils_messenger_create_info, nullptr, &messenger) != VK_SUCCESS || messenger == VK_NULL_HANDLE) {
			throw Err::Vulkan;
		}
		furnace.debug_messenger = messenger;
	}
#endif
	assert(furnace.physical_device == VK_NULL_HANDLE);
	{
		u32 physical_device_count = 0;
		if (vkEnumeratePhysicalDevices(furnace.instance, &physical_device_count, nullptr) != VK_SUCCESS) {
			throw Err::Vulkan;
		}
		std::vector<VkPhysicalDevice>physical_devices{ physical_device_count };
		if (vkEnumeratePhysicalDevices(furnace.instance, &physical_device_count, physical_devices.data()) != VK_SUCCESS) {
			throw Err::Vulkan;
		}

		for (VkPhysicalDevice physical_device : physical_devices) {
			{
				VkPhysicalDeviceFeatures supported_features;
				vkGetPhysicalDeviceFeatures(physical_device, &supported_features);
				if (!supported_features.samplerAnisotropy || !supported_features.sampleRateShading) {
					continue;
				}
			}
			{
				u32 device_extension_count;
				if (vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, nullptr) != VK_SUCCESS) {
					throw Err::Vulkan;
				}
				std::vector<VkExtensionProperties> device_extensions{ device_extension_count };
				if (vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, device_extensions.data()) != VK_SUCCESS) {
					throw Err::Vulkan;
				}
				bool found_all = true;
				for (const char* required : Furnace::RequiredDeviceExtensions) {
					bool found = false;
					for (const auto& extension : device_extensions) {
						found = strcmp(required, extension.extensionName) == 0;
						if (found) {
							break;
						}
					}
					if (!found) {
						found_all = false;
						break;
					}
				}
				if (!found_all) {
					continue;
				}
			}
			u32 queue_family = UINT32_MAX;
			{
				u32 queue_family_count = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
				std::vector<VkQueueFamilyProperties> queue_families{ queue_family_count };
				vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

				bool found = false;
				for (u32 i = 0; i < queue_family_count; i++) {
					if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT) {
						queue_family = i;
						found = true;
						break;
					}
				}
				if (!found) {
					continue;
				}
			}
			furnace.physical_device = physical_device;
			furnace.queue_family = queue_family;
		}
		if (furnace.physical_device == VK_NULL_HANDLE) throw Err::Vulkan;
	}
	assert(furnace.device == VK_NULL_HANDLE);
	{
		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
			.dynamicRendering = VK_TRUE
		};
		VkPhysicalDeviceFeatures device_features{
			.sampleRateShading = VK_TRUE,
			.samplerAnisotropy = VK_TRUE,
		};
		float queue_priority = 1.0f;
		VkDeviceQueueCreateInfo queue_info{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = furnace.queue_family,
			.queueCount = 1,
			.pQueuePriorities = &queue_priority,
		};
		VkDeviceCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &dynamic_rendering_feature,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queue_info,
	#ifdef NDEBUG
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = nullptr,
	#else
			.enabledLayerCount = static_cast<u32>(Furnace::ValidationLayers.size()),
			.ppEnabledLayerNames = Furnace::ValidationLayers.data(),
	#endif
			.enabledExtensionCount = static_cast<u32>(Furnace::RequiredDeviceExtensions.size()),
			.ppEnabledExtensionNames = Furnace::RequiredDeviceExtensions.data(),
			.pEnabledFeatures = &device_features
		};
		VkDevice device = VK_NULL_HANDLE;
		if (vkCreateDevice(furnace.physical_device, &create_info, nullptr, &device) != VK_SUCCESS || device == VK_NULL_HANDLE) {
			throw Err::Vulkan;
		}
		furnace.device = device;
	}
	assert(furnace.queue == VK_NULL_HANDLE);
	{
		VkQueue queue = VK_NULL_HANDLE;
		vkGetDeviceQueue(furnace.device, furnace.queue_family, 0, &queue);
		if (queue == VK_NULL_HANDLE) {
			throw Err::Vulkan;
		}
		furnace.queue = queue;
	}
	assert(furnace.command_pool == VK_NULL_HANDLE);
	{
		VkCommandPoolCreateInfo pool_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = furnace.queue_family,
		};
		VkCommandPool cmd_pool = VK_NULL_HANDLE;
		if (vkCreateCommandPool(furnace.device, &pool_info, nullptr, &cmd_pool) != VK_SUCCESS || cmd_pool == VK_NULL_HANDLE) throw Err::Vulkan;
		furnace.command_pool = cmd_pool;
	};
}

FurnaceKeeper::~FurnaceKeeper() noexcept {
	Furnace& furnace = get();
	if (furnace.device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(furnace.device);
	}
	if (furnace.command_pool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(furnace.device, furnace.command_pool, nullptr);
		furnace.command_pool = VK_NULL_HANDLE;
	}
	if (furnace.device != VK_NULL_HANDLE) {
		vkDestroyDevice(furnace.device, nullptr);
		furnace.device = VK_NULL_HANDLE;
	}
	furnace.physical_device = VK_NULL_HANDLE;
	furnace.queue_family = UINT32_MAX;
#ifndef NDEBUG
	if (furnace.debug_messenger != VK_NULL_HANDLE) {
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(furnace.instance, "vkDestroyDebugUtilsMessengerEXT");
		vkDestroyDebugUtilsMessengerEXT(furnace.instance, furnace.debug_messenger, nullptr);
		furnace.debug_messenger = VK_NULL_HANDLE;
	}
#endif
	if (furnace.instance != VK_NULL_HANDLE) {
		vkDestroyInstance(furnace.instance, nullptr);
		furnace.instance = VK_NULL_HANDLE;
	}
	if (furnace.SDL_initialized) {
		SDL_Quit();
		furnace.SDL_initialized = false;
	}
}

std::optional<VkFormat> find_supported_format(std::span<VkFormat> candidates, const VkImageTiling tiling, const VkFormatFeatureFlags features) noexcept {
	for (auto candidate : candidates) {
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(FurnaceKeeper::physicalDevice(), candidate, &properties);
		if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
			return candidate;
		}
		/*else if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return candidate
		}*/
	}
	return {};
}

std::optional<u32> find_memory_type_index(const u32 type_filter, const VkMemoryPropertyFlags properties) noexcept {
	VkPhysicalDeviceMemoryProperties mem_props;
	vkGetPhysicalDeviceMemoryProperties(FurnaceKeeper::physicalDevice(), &mem_props);
	for (u32 i = 0; i < mem_props.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return {};
}

VkFormat get_depth_format() {
	static VkFormat depth_format = VK_FORMAT_UNDEFINED;
	if (depth_format != VK_FORMAT_UNDEFINED) {
		return depth_format;
	}
	VkFormat depth_format_candidates[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	if (auto res = find_supported_format(depth_format_candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT); !res) {
		throw Err::NoResult;
	} else {
		depth_format = res.value();
		return depth_format;
	}
}

VkSampler createSampler(VkFilter type) {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(FurnaceKeeper::physicalDevice(), &properties);
	VkSamplerCreateInfo sampler_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = type,
		.minFilter = type,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = 0.0f,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = 0.0f,
		.maxLod = 0.0f,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE
	};
	VkSampler sampler = VK_NULL_HANDLE;
	if (vkCreateSampler(FurnaceKeeper::device(), &sampler_info, nullptr, &sampler) != VK_SUCCESS || sampler == VK_NULL_HANDLE) {
		throw Err::Vulkan;
	}
	return sampler;
}

Buffer Buffer::create(const CreateInfo& create_info) {
	Buffer buffer;

	VkBufferCreateInfo buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = create_info.size,
		.usage = create_info.usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &FurnaceKeeper::get().queue_family,
	};
	if (create_info.memory_type == MemoryType::DeviceLocal) {
		buffer_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}
	if (vkCreateBuffer(FurnaceKeeper::device(), &buffer_info, nullptr, &buffer.handle) != VK_SUCCESS || buffer.handle == VK_NULL_HANDLE) {
		throw Err::Vulkan;
	}

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(FurnaceKeeper::device(), buffer.handle, &mem_reqs);
	u32 memory_type_index;
	if (auto res = find_memory_type_index(mem_reqs.memoryTypeBits, static_cast<VkMemoryPropertyFlags>(create_info.memory_type)); !res) {
		throw Err::Vulkan;
	} else memory_type_index = res.value();
	VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = mem_reqs.size,
		.memoryTypeIndex = memory_type_index
	};
	if (vkAllocateMemory(FurnaceKeeper::device(), &alloc_info, nullptr, &buffer.memory) || buffer.memory == VK_NULL_HANDLE) {
		throw Err::Vulkan;
	}
	if (vkBindBufferMemory(FurnaceKeeper::device(), buffer.handle, buffer.memory, 0)) {
		throw Err::Vulkan;
	}

	return buffer;
}

void Buffer::writeHostVisible(const void* data, VkDeviceSize size, VkDeviceSize offset) const {
	void* map;
	if (vkMapMemory(Vk::FurnaceKeeper::device(), memory, offset, size, 0, &map) != VK_SUCCESS) {
		throw Err::Vulkan;
	}
	memcpy((u8*)map + offset, data, size);
	vkUnmapMemory(Vk::FurnaceKeeper::device(), memory);
}

void Buffer::writeDeviceLocal(const void* data, VkDeviceSize size, VkDeviceSize offset) {
	Vk::Buffer staging_buffer = Vk::Buffer::create({
		.size = size,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.memory_type = Vk::MemoryType::HostVisible,
		});
	staging_buffer.writeHostVisible(data, size, offset);

	VkCommandBuffer cmd_buf = Reflect::beginOneTimeCommand();
	Reflect::cmdCopyBufferToBuffer(cmd_buf, staging_buffer, offset, size, *this, offset);
	Reflect::endAndSubmitOneTimeCommand(cmd_buf);
}

Image Image::create(const CreateInfo& create_info) {
	Image image;
	{
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
			.pQueueFamilyIndices = &FurnaceKeeper::get().queue_family,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};
		if (create_info.memory_type == MemoryType::DeviceLocal) {
			image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		if (vkCreateImage(FurnaceKeeper::device(), &image_info, nullptr, &image.handle) != VK_SUCCESS || image.handle == VK_NULL_HANDLE) {
			throw Err::Vulkan;
		}
	}
	{
		VkMemoryRequirements mem_reqs;
		vkGetImageMemoryRequirements(FurnaceKeeper::device(), image.handle, &mem_reqs);
		u32 memory_type_index;
		if (auto res = find_memory_type_index(mem_reqs.memoryTypeBits, static_cast<VkMemoryPropertyFlags>(create_info.memory_type)); !res) {
			throw Err::Vulkan;
		} else memory_type_index = res.value();
		VkMemoryAllocateInfo alloc_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = mem_reqs.size,
			.memoryTypeIndex = memory_type_index,
		};
		if (vkAllocateMemory(FurnaceKeeper::device(), &alloc_info, nullptr, &image.memory) != VK_SUCCESS || image.memory == VK_NULL_HANDLE) {
			throw Err::Vulkan;
		}
		if (vkBindImageMemory(FurnaceKeeper::device(), image.handle, image.memory, 0) != VK_SUCCESS) {
			throw Err::Vulkan;
		}
	}
	{
		VkImageViewCreateInfo view_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = image.handle,
			.viewType = static_cast<VkImageViewType>(create_info.image_type),
			.format = create_info.format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_R,
				.g = VK_COMPONENT_SWIZZLE_G,
				.b = VK_COMPONENT_SWIZZLE_B,
				.a = VK_COMPONENT_SWIZZLE_A
			},
			.subresourceRange = {
				.aspectMask = create_info.aspect_mask,
				.baseMipLevel = 0,
				.levelCount = create_info.mip_levels,
				.baseArrayLayer = 0,
				.layerCount = create_info.array_layers,
			},
		};
		if (vkCreateImageView(FurnaceKeeper::device(), &view_info, nullptr, &image.view) != VK_SUCCESS || image.view == VK_NULL_HANDLE) {
			throw Err::Vulkan;
		}
	}
	return image;
}

void Image::writeHostVisible(const void* data, const VkExtent3D extent, const u32 pixel_alignment) const {
	void* map;
	VkDeviceSize size = (VkDeviceSize)extent.width * (VkDeviceSize)extent.height * (VkDeviceSize)extent.depth * (VkDeviceSize)pixel_alignment;
	if (vkMapMemory(Vk::FurnaceKeeper::device(), memory, 0, size, 0, &map) != VK_SUCCESS) {
		throw Err::Vulkan;
	}
	memcpy((u8*)map, data, size);
	vkUnmapMemory(Vk::FurnaceKeeper::device(), memory);
}

void Image::writeDeviceLocal(const void* data, const VkExtent3D extent, const u32 pixel_alignment, const VkImageLayout final_layout) {
	VkDeviceSize size = (VkDeviceSize)extent.width * (VkDeviceSize)extent.height * (VkDeviceSize)extent.depth * (VkDeviceSize)pixel_alignment;
	Vk::Buffer staging_buffer = Vk::Buffer::create({
		.size = size,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.memory_type = Vk::MemoryType::HostVisible,
		});
	staging_buffer.writeHostVisible(data, size, 0);

	VkCommandBuffer command_buffer = Reflect::beginOneTimeCommand();
	Reflect::cmdInsertImageMemoryBarrier(command_buffer, handle,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT, 1
		);
	Reflect::cmdCopyBufferToImage(command_buffer, staging_buffer, extent, *this);
	Reflect::cmdInsertImageMemoryBarrier(command_buffer, handle,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, final_layout,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT, 1
		);
	Reflect::endAndSubmitOneTimeCommand(command_buffer);
}

DescriptorPool DescriptorPool::create(const CreateInfo& create_info) {
	DescriptorPool pool;

	for (const auto& set : create_info.sets) {
		VkDescriptorSetLayoutCreateInfo layout_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<u32>(set.bindings.size()),
			.pBindings = set.bindings.data(),
		};
		VkDescriptorSetLayout layout = VK_NULL_HANDLE;
		if (vkCreateDescriptorSetLayout(FurnaceKeeper::device(), &layout_info, nullptr, &layout) != VK_SUCCESS || layout == VK_NULL_HANDLE) {
			throw Err::Vulkan;
		}
		pool.layouts.push_back(layout);
	}

	u32 max_sets = 0;
	std::vector<VkDescriptorPoolSize> sizes;
	for (const auto& set : create_info.sets) {
		max_sets += set.count;
		for (auto& binding : set.bindings) {
			auto it = std::find_if(sizes.begin(), sizes.end(), [type = binding.descriptorType](const VkDescriptorPoolSize size) {
				return type == size.type;
			});
			if (it != sizes.end()) {
				it->descriptorCount += binding.descriptorCount * set.count;
			} else {
				sizes.emplace_back(static_cast<VkDescriptorType>(binding.descriptorType), binding.descriptorCount * set.count);
			}
		}
	}

	VkDescriptorPoolCreateInfo pool_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			.maxSets = max_sets,
			.poolSizeCount = static_cast<u32>(sizes.size()),
			.pPoolSizes = sizes.data(),
	};
	if (vkCreateDescriptorPool(FurnaceKeeper::device(), &pool_info, nullptr, &pool.handle) != VK_SUCCESS || pool.handle == VK_NULL_HANDLE) {
		throw Err::Vulkan;
	}

	return pool;
}

void DescriptorPool::allocateSets(const std::span<const VkDescriptorSetLayout> set_layouts, const std::span<VkDescriptorSet> out_sets) const {
	assert(set_layouts.size() == out_sets.size());
	VkDescriptorSetAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = handle,
		.descriptorSetCount = static_cast<u32>(set_layouts.size()),
		.pSetLayouts = set_layouts.data(),
	};
	if (vkAllocateDescriptorSets(FurnaceKeeper::device(), &alloc_info, out_sets.data()) != VK_SUCCESS || out_sets[0] == VK_NULL_HANDLE) {
		throw Err::Vulkan;
	}
}

void writeDescriptorSetBuffer(VkDescriptorSet set, const uint32_t binding, const VkBuffer buffer, const VkDeviceSize range, const VkDeviceSize offset) noexcept {
	VkDescriptorBufferInfo buffer_info{ buffer, offset, range };
	VkWriteDescriptorSet descriptor_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = set,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pBufferInfo = &buffer_info
	};
	vkUpdateDescriptorSets(Vk::FurnaceKeeper::device(), 1, &descriptor_write, 0, nullptr);
}

void writeDescriptorSetImage(VkDescriptorSet set, const uint32_t binding, const VkSampler sampler, const VkImageView view, const VkImageLayout layout) noexcept {
	VkDescriptorImageInfo image_info{ sampler, view, layout };
	VkWriteDescriptorSet descriptor_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = set,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = &image_info,
	};
	vkUpdateDescriptorSets(Vk::FurnaceKeeper::device(), 1, &descriptor_write, 0, nullptr);
}

[[nodiscard]] Pipeline Pipeline::createGraphics(const CreateInfo& create_info) {
	Pipeline pipeline;

	std::array shader_paths{ create_info.vertex_shader, create_info.fragment_shader };
	for (const auto& path : shader_paths) {
		auto file = std::ifstream{ path.data(), std::ios::ate | std::ios::binary };
		if (!file.is_open()) {
			throw Err::CouldNotOpenFile;
		}
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
		if (vkCreateShaderModule(FurnaceKeeper::device(), &shader_info, nullptr, &shader) != VK_SUCCESS || shader == VK_NULL_HANDLE) {
			throw Err::Vulkan;
		}
		pipeline.shader_modules.push_back(shader);
	}

	VkPipelineLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = static_cast<u32>(create_info.descriptor_layouts.size()),
		.pSetLayouts = create_info.descriptor_layouts.data(),
		.pushConstantRangeCount = create_info.push_constant.has_value() ? 1U : 0U,
		.pPushConstantRanges = create_info.push_constant.has_value() ? &create_info.push_constant.value() : nullptr,
	};
	if (vkCreatePipelineLayout(FurnaceKeeper::device(), &layout_info, nullptr, &pipeline.layout) != VK_SUCCESS || pipeline.layout == VK_NULL_HANDLE) {
		throw Err::Vulkan;
	}

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
			.module = pipeline.shader_modules[0],
			.pName = "main",
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = pipeline.shader_modules[1],
			.pName = "main",
		}
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
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f,
	};
	VkPipelineMultisampleStateCreateInfo multisample_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
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
		.front = {},							// fix
		.back = {},								// fix
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
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};
	VkPipelineColorBlendStateCreateInfo color_blend_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment,
		.blendConstants = { 1.0f, 1.0f, 1.0f, 1.0f },
	};
	VkDynamicState dynamic_state_enables[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamic_state_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = std::size(dynamic_state_enables),
		.pDynamicStates = dynamic_state_enables,
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
		.layout = pipeline.layout,
		.renderPass = nullptr,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};
	if (vkCreateGraphicsPipelines(FurnaceKeeper::device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline.handle) != VK_SUCCESS || pipeline.handle == VK_NULL_HANDLE) {
		throw Err::Vulkan;
	}
	return pipeline;
}

Swapchain Swapchain::create(const Vec2<i32> window_size, const Surface& surface, const VkSwapchainKHR old_swapchain) {
	Swapchain swapchain;

	VkSurfaceCapabilitiesKHR surface_capabilities;
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(FurnaceKeeper::physicalDevice(), surface.handle, &surface_capabilities) != VK_SUCCESS) {
		throw Err::Vulkan;
	}
	Vec2<u32> extent = { static_cast<u32>(window_size.x), static_cast<u32>(window_size.y) };
	{
		if (surface_capabilities.currentExtent.width != UINT32_MAX) {
			extent.x = surface_capabilities.currentExtent.width;
			extent.y = surface_capabilities.currentExtent.height;
		} else {
			extent.x = std::max(surface_capabilities.minImageExtent.width, std::min(surface_capabilities.maxImageExtent.width, extent.x));
			extent.y = std::max(surface_capabilities.minImageExtent.height, std::min(surface_capabilities.maxImageExtent.height, extent.y));
		}
	}
	assert(extent.x > 0 && extent.y > 0);

	u32 img_count;
	{
		img_count = surface_capabilities.minImageCount + 1;
		if (surface_capabilities.maxImageCount > 0 && img_count > surface_capabilities.maxImageCount) {
			img_count = surface_capabilities.maxImageCount;
		}
		if (img_count == 0) throw Err::Vulkan;
	}

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	{
		uint32_t present_mode_count;
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(FurnaceKeeper::physicalDevice(), surface.handle, &present_mode_count, nullptr) != VK_SUCCESS) {
			throw Err::Vulkan;
		}
		std::vector<VkPresentModeKHR> available_present_modes{ 32 };
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(FurnaceKeeper::physicalDevice(), surface.handle, &present_mode_count, available_present_modes.data()) != VK_SUCCESS) {
		}
		for (auto available_present_mode : available_present_modes) {
			if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				present_mode = available_present_mode;
				break;
			}
		}
	}

	VkSwapchainCreateInfoKHR create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface.handle,
		.minImageCount = img_count,
		.imageFormat = surface.Format.format,
		.imageColorSpace = surface.Format.colorSpace,
		.imageExtent = { extent.x, extent.y },
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &FurnaceKeeper::get().queue_family,
		.preTransform = surface_capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.clipped = VK_TRUE,
		.oldSwapchain = old_swapchain,
	};
	if (vkCreateSwapchainKHR(FurnaceKeeper::device(), &create_info, nullptr, &swapchain.handle) != VK_SUCCESS || swapchain.handle == VK_NULL_HANDLE) {
		throw Err::Vulkan;
	}

	if (vkGetSwapchainImagesKHR(FurnaceKeeper::device(), swapchain.handle, &swapchain.image_count, nullptr) != VK_SUCCESS || swapchain.image_count == 0) {
		throw Err::Vulkan;
	}
	if (vkGetSwapchainImagesKHR(FurnaceKeeper::device(), swapchain.handle, &swapchain.image_count, swapchain.images.data()) != VK_SUCCESS) {
		throw Err::Vulkan;
	}
	for (usize i = 0; i < swapchain.image_count; i++) {
		VkImageViewCreateInfo view_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapchain.images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = Surface::Format.format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_R,
				.g = VK_COMPONENT_SWIZZLE_G,
				.b = VK_COMPONENT_SWIZZLE_B,
				.a = VK_COMPONENT_SWIZZLE_A
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};
		if (vkCreateImageView(FurnaceKeeper::device(), &view_info, nullptr, &swapchain.image_views[i]) != VK_SUCCESS || swapchain.image_views[i] == VK_NULL_HANDLE) {
			throw Err::Vulkan;
		}
	}
	return swapchain;
}

}

namespace Mirror::Reflect {

void allocateCommandBuffers(const VkQueueFlags type, std::span<VkCommandBuffer> cmd_bufs) {
	assert(type & VK_QUEUE_GRAPHICS_BIT);

	VkCommandBufferAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = Vk::FurnaceKeeper::commandPool(),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = (u32)cmd_bufs.size(),
	};
	if (vkAllocateCommandBuffers(Vk::FurnaceKeeper::device(), &alloc_info, cmd_bufs.data()) != VK_SUCCESS || cmd_bufs[0] == VK_NULL_HANDLE) throw Err::Vulkan;
}


VkCommandBuffer beginOneTimeCommand() {
	VkCommandBuffer command_buffer{};
	allocateCommandBuffers(VK_QUEUE_GRAPHICS_BIT, { &command_buffer, 1 });

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(command_buffer, &begin_info);
	return command_buffer;
}

void endAndSubmitOneTimeCommand(VkCommandBuffer command_buffer) {
	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) throw Err::Vulkan;
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer,
	};
	if (vkQueueSubmit(Vk::FurnaceKeeper::queue(), 1, &submit_info, nullptr) != VK_SUCCESS) throw Err::Vulkan;
	if (vkQueueWaitIdle(Vk::FurnaceKeeper::queue()) != VK_SUCCESS) throw Err::Vulkan;
	vkFreeCommandBuffers(Vk::FurnaceKeeper::device(), Vk::FurnaceKeeper::commandPool(), 1, &command_buffer);
}

void cmdCopyBufferToBuffer(const VkCommandBuffer cmd_buf, const Vk::Buffer& src_buffer, const VkDeviceSize src_offset, const VkDeviceSize size, Vk::Buffer& dst_buffer, const VkDeviceSize dst_offset) noexcept {
	VkBufferCopy copy_region = {
		.srcOffset = src_offset,
		.dstOffset = dst_offset,
		.size = size,
	};
	vkCmdCopyBuffer(cmd_buf, src_buffer.handle, dst_buffer.handle, 1, &copy_region);
}

void cmdCopyBufferToImage(const VkCommandBuffer command_buffer, const Vk::Buffer& src_buffer, const VkExtent3D extent, Vk::Image& dst_image) noexcept {
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
	vkCmdCopyBufferToImage(command_buffer, src_buffer.handle, dst_image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void cmdInsertImageMemoryBarrier(const VkCommandBuffer command_buffer, 
	const VkImage image,
	const VkImageLayout old_layout,
	const VkImageLayout new_layout,
	const VkPipelineStageFlags src_stage, 
	const VkPipelineStageFlags dst_stage,
	const VkAccessFlags src_access_mask,
	const VkAccessFlags dst_access_mask,
	VkImageAspectFlags aspect_mask,
	u32 mip_level
) noexcept {
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
	vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

}

