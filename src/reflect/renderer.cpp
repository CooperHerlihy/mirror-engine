#include "reflect/renderer.h"

namespace Mirror::Reflect {

Renderer::Renderer(const std::string_view app_name, const Vec2<i32> window_size) :
	window{ SDL_CreateWindow(app_name.data(), window_size.x, window_size.y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN) }
{
	Vk::allocate_command_buffers({ reinterpret_cast<VkCommandBuffer*>(&command_buffers), MaxFramesInFlight });
	for (auto& semaphore : image_available_semaphores) {
		semaphore = Vk::create_semaphore();
	}
	for (auto& semaphore : render_finished_semaphores) {
		semaphore = Vk::create_semaphore();
	}
	for (auto& fence : in_flight_fences) {
		fence = Vk::create_fence(VK_FENCE_CREATE_SIGNALED_BIT);
	}
	Vk::writeToHostVisibleMemory(vp_buffer.memory.handle(), &vp_data, sizeof(vp_data));
	Vk::write_descriptor_set_buffer(vp_set, 0, vp_buffer.handle(), sizeof(vp_data));
}

void Renderer::update() {
	vp_data.view = camera.view();
	Vk::writeToHostVisibleMemory(vp_buffer.memory.handle(), &vp_data, sizeof(vp_data));

	Vk::wait_for_fence(in_flight_fences[current_frame].handle());
	vkAcquireNextImageKHR(Vk::device(), swapchain.handle(), UINT64_MAX, image_available_semaphores[current_frame].handle(), VK_NULL_HANDLE, &swapchain.current_image);

	VkExtent3D extent = get_window_extent();
	Vk::CommandBuffer& cmd = command_buffers[current_frame];
	Vk::cmd_begin(cmd.handle());
	Vk::cmd_set_viewport_and_scissor(cmd.handle(), extent);

	Vk::cmd_insert_image_memory_barrier(cmd.handle(), swapchain.images[swapchain.current_image],
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT
		);
	Vk::cmd_insert_image_memory_barrier(cmd.handle(), color_image.handle(),
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		);

	VkRenderingAttachmentInfo color_attachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = color_image.view.handle(),
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		//.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = { 0.0f, 0.0f, 0.0f, 1.0f },
	};
	VkRenderingAttachmentInfo depth_attachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = depth_image.view.handle(),
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.clearValue = { 1.0f, 1.0f, 1.0f, 1.0f },
	};
	VkRenderingInfo rendering_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = { { 0, 0 }, { extent.width, extent.height } },
		.layerCount = 1,
		.viewMask = 0,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment,
		.pDepthAttachment = &depth_attachment,
	};
	vkCmdBeginRendering(cmd.handle(), &rendering_info);

	sprite_manager.cmd_render(cmd.handle(), vp_set);

	vkCmdEndRendering(cmd.handle());

	Vk::cmd_insert_image_memory_barrier(cmd.handle(), color_image.handle(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT
		);
	VkImageResolve resolve{
		.srcSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.dstSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.extent = extent,
	};
	vkCmdResolveImage(cmd.handle(), color_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchain.images[swapchain.current_image], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolve);

	Vk::cmd_insert_image_memory_barrier(cmd.handle(), swapchain.images[swapchain.current_image],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE
		);

	vkEndCommandBuffer(cmd.handle());

	if (images_in_flight[swapchain.current_image] != VK_NULL_HANDLE) {
		Vk::wait_for_fence(images_in_flight[swapchain.current_image]);
	}
	images_in_flight[swapchain.current_image] = in_flight_fences[current_frame].handle();
	vkResetFences(Vk::device(), 1, in_flight_fences[current_frame].ptr());

	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = image_available_semaphores[current_frame].ptr(),
		.pWaitDstStageMask = &wait_stage,
		.commandBufferCount = 1,
		.pCommandBuffers = cmd.ptr(),
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = render_finished_semaphores[current_frame].ptr(),
	};
	vkQueueSubmit(Vk::queue(), 1, &submit_info, in_flight_fences[current_frame].handle());

	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = render_finished_semaphores[current_frame].ptr(),
		.swapchainCount = 1,
		.pSwapchains = swapchain.ptr(),
		.pImageIndices = &swapchain.current_image,
	};
	vkQueuePresentKHR(Vk::queue(), &present_info);

	current_frame = (current_frame + 1) % MaxFramesInFlight;
}

}

