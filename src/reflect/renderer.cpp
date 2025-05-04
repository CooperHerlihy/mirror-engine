#include "reflect/renderer.h"

namespace Mirror::Reflect {

Renderer::Renderer(const std::string_view app_name, const Vec2<i32> window_size)
	: m_window{
		  SDL_CreateWindow(app_name.data(), window_size.x, window_size.y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN)} {
	debug_assert(m_window != nullptr);
	debug_assert(m_surface.valid());
	debug_assert(m_swapchain.valid());
	debug_assert(m_color_image.valid());
	debug_assert(m_depth_image.valid());
	debug_assert(m_nearest_sampler.valid());
	debug_assert(m_linear_sampler.valid());
	debug_assert(m_vp_buffer.valid());
	debug_assert(m_vp_descriptor_pool.valid());
	debug_assert(m_vp_set != VK_NULL_HANDLE);

	static_assert(sizeof(VkCommandBuffer) == sizeof(Vk::CommandBuffer));
	Vk::allocate_command_buffers({reinterpret_cast<VkCommandBuffer*>(&m_command_buffers), MaxFramesInFlight});
	for (const auto& cmd : m_command_buffers) {
		debug_assert(cmd.valid());
	}
	for (auto& semaphore : m_image_available_semaphores) {
		semaphore = Vk::create_semaphore();
		debug_assert(semaphore.valid());
	}
	for (auto& semaphore : m_render_finished_semaphores) {
		semaphore = Vk::create_semaphore();
		debug_assert(semaphore.valid());
	}
	for (auto& fence : m_in_flight_fences) {
		fence = Vk::create_fence(VK_FENCE_CREATE_SIGNALED_BIT);
		debug_assert(fence.valid());
	}
	VPUniform vp_data;
	Vk::write_to_host_visible_memory(m_vp_buffer.memory.handle(), &vp_data, sizeof(vp_data));
	Vk::write_descriptor_set_buffer(m_vp_set, 0, m_vp_buffer.handle(), sizeof(vp_data));
}

void Renderer::update() {
	Vk::wait_for_fence(m_in_flight_fences[m_current_frame].handle());
	vkAcquireNextImageKHR(Vk::device(), m_swapchain.handle(), UINT64_MAX,
						  m_image_available_semaphores[m_current_frame].handle(), VK_NULL_HANDLE, &m_swapchain.current_image);

	VkExtent3D extent = get_window_extent();
	Vk::CommandBuffer& cmd = m_command_buffers[m_current_frame];
	debug_assert(cmd.valid());

	Vk::cmd_begin(cmd.handle());
	Vk::cmd_set_viewport_and_scissor(cmd.handle(), extent);

	Vk::cmd_insert_image_memory_barrier(cmd.handle(), m_swapchain.images[m_swapchain.current_image],
										VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
										VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
										VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT);
	Vk::cmd_insert_image_memory_barrier(cmd.handle(), m_color_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED,
										VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
										VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_NONE,
										VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

	debug_assert(m_color_image.valid());
	VkRenderingAttachmentInfo color_attachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = m_color_image.view.handle(),
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		//.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = {0.0f, 0.0f, 0.0f, 1.0f},
	};
	debug_assert(m_depth_image.valid());
	VkRenderingAttachmentInfo depth_attachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = m_depth_image.view.handle(),
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.clearValue = {1.0f, 1.0f, 1.0f, 1.0f},
	};
	VkRenderingInfo rendering_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = {{0, 0}, {extent.width, extent.height}},
		.layerCount = 1,
		.viewMask = 0,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment,
		.pDepthAttachment = &depth_attachment,
	};
	vkCmdBeginRendering(cmd.handle(), &rendering_info);

	debug_assert(m_vp_set != VK_NULL_HANDLE);
	m_sprite_manager.cmd_render(cmd.handle(), m_vp_set);
	m_model_manager.cmd_render(cmd.handle(), m_vp_set);

	vkCmdEndRendering(cmd.handle());

	Vk::cmd_insert_image_memory_barrier(cmd.handle(), m_color_image.handle(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
										VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
										VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
										VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
	VkImageResolve resolve{
		.srcSubresource
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		.dstSubresource
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		.extent = extent,
	};
	vkCmdResolveImage(cmd.handle(), m_color_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					  m_swapchain.images[m_swapchain.current_image], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolve);

	Vk::cmd_insert_image_memory_barrier(cmd.handle(), m_swapchain.images[m_swapchain.current_image],
										VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
										VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
										VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE);

	vkEndCommandBuffer(cmd.handle());

	if (m_images_in_flight[m_swapchain.current_image] != VK_NULL_HANDLE) {
		Vk::wait_for_fence(m_images_in_flight[m_swapchain.current_image]);
	}
	debug_assert(m_in_flight_fences[m_current_frame].valid());
	m_images_in_flight[m_swapchain.current_image] = m_in_flight_fences[m_current_frame].handle();
	vkResetFences(Vk::device(), 1, m_in_flight_fences[m_current_frame].ptr());

	debug_assert(m_image_available_semaphores[m_current_frame].valid());
	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = m_image_available_semaphores[m_current_frame].ptr(),
		.pWaitDstStageMask = &wait_stage,
		.commandBufferCount = 1,
		.pCommandBuffers = cmd.ptr(),
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = m_render_finished_semaphores[m_current_frame].ptr(),
	};
	vkQueueSubmit(Vk::queue(), 1, &submit_info, m_in_flight_fences[m_current_frame].handle());

	debug_assert(m_render_finished_semaphores[m_current_frame].valid());
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = m_render_finished_semaphores[m_current_frame].ptr(),
		.swapchainCount = 1,
		.pSwapchains = m_swapchain.ptr(),
		.pImageIndices = &m_swapchain.current_image,
	};
	vkQueuePresentKHR(Vk::queue(), &present_info);

	m_current_frame = (m_current_frame + 1) % MaxFramesInFlight;
}

bool Renderer::resize() {
	Vec2<i32> size = get_window_size();
	if (size.x <= 2 || size.y <= 2) {
		return false;
	}
	Vk::wait_for_fences(m_images_in_flight);
	m_swapchain = Vk::Swapchain::create(size, m_surface.handle(), m_swapchain.handle());
	release_assert(m_swapchain.valid());
	m_color_image = Vk::Image::create({
		.extent = get_window_extent(),
		.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		.memory_type = Vk::MemoryType::DeviceLocal,
		.format = Vk::SurfaceFormat.format,
		.samples = VK_SAMPLE_COUNT_4_BIT,
	});
	release_assert(m_color_image.valid());
	m_depth_image = Vk::Image::create({
		.extent = get_window_extent(),
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.memory_type = Vk::MemoryType::DeviceLocal,
		.format = Vk::get_depth_format(),
		.aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT,
		.samples = VK_SAMPLE_COUNT_4_BIT,
	});
	release_assert(m_depth_image.valid());
	return true;
};

} // namespace Mirror::Reflect
