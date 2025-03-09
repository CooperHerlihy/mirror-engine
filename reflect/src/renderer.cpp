#include "renderer.h"

namespace Mirror::Reflect {

Renderer::Renderer(const std::string_view app_name, const Vec2<i32> window_size) :
	window{ SDL_CreateWindow(app_name.data(), window_size.x, window_size.y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN) }
{
	Vk::CommandBuffer::allocate(VK_QUEUE_GRAPHICS_BIT, { reinterpret_cast<VkCommandBuffer*>(&command_buffers), MaxFramesInFlight });
	for (auto& semaphore : image_available_semaphores) {
		semaphore = Vk::createSemaphore();
	}
	for (auto& semaphore : render_finished_semaphores) {
		semaphore = Vk::createSemaphore();
	}
	for (auto& fence : in_flight_fences) {
		fence = Vk::createFence();
	}
	vp_buffer.writeHostVisible(&vp_data, sizeof(vp_data));
	Vk::writeDescriptorSetBuffer(vp_set, 0, vp_buffer.handle, sizeof(vp_data));
}

void Renderer::update() {
	vp_data.view = camera.view();
	vp_buffer.writeHostVisible(&vp_data, sizeof(vp_data));

	Vk::waitForFence(in_flight_fences[current_frame]);
	vkAcquireNextImageKHR(Vk::FurnaceKeeper::device(), swapchain, UINT64_MAX, image_available_semaphores[current_frame].handle(), VK_NULL_HANDLE, &swapchain.current_image);

	Vk::CommandBuffer& cmd = command_buffers[current_frame];
	cmd.begin();
	cmd.setViewportAndScissor(get_window_size());

	cmdInsertImageMemoryBarrier(cmd.handle, swapchain.images[swapchain.current_image],
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT, 1);

	Vec3<i32> extent = get_window_size();
	VkRenderingAttachmentInfo color_attachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = swapchain.image_views[swapchain.current_image],
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.resolveMode = VK_RESOLVE_MODE_NONE,
		.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = { 0.0f, 0.0f, 0.0f, 1.0f },
	};
	VkRenderingInfo rendering_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = { { 0, 0 }, { static_cast<u32>(extent.x), static_cast<u32>(extent.y) } },
		.layerCount = 1,
		.viewMask = 0,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment,
	};
	vkCmdBeginRendering(cmd.handle, &rendering_info);

	sprite_manager.cmdRender(cmd.handle, vp_set);

	vkCmdEndRendering(cmd.handle);

	cmdInsertImageMemoryBarrier(cmd.handle, swapchain.images[swapchain.current_image],
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_NONE,
		VK_IMAGE_ASPECT_COLOR_BIT, 1);

	vkEndCommandBuffer(cmd.handle);

	if (images_in_flight[swapchain.current_image] != VK_NULL_HANDLE) {
		vkWaitForFences(Vk::FurnaceKeeper::device(), 1, &images_in_flight[swapchain.current_image], VK_TRUE, UINT64_MAX);
	}
	images_in_flight[swapchain.current_image] = in_flight_fences[current_frame].handle();
	vkResetFences(Vk::FurnaceKeeper::device(), 1, in_flight_fences[current_frame].ptr());

	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = image_available_semaphores[current_frame].ptr(),
		.pWaitDstStageMask = &wait_stage,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd.handle,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = render_finished_semaphores[current_frame].ptr(),
	};
	vkQueueSubmit(Vk::FurnaceKeeper::queue(), 1, &submit_info, in_flight_fences[current_frame].handle());

	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = render_finished_semaphores[current_frame].ptr(),
		.swapchainCount = 1,
		.pSwapchains = &swapchain.handle,
		.pImageIndices = &swapchain.current_image,
	};
	vkQueuePresentKHR(Vk::FurnaceKeeper::queue(), &present_info);

	current_frame = (current_frame + 1) % MaxFramesInFlight;
}

}

