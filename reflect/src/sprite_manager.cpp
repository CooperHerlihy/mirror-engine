#include "sprite_manager.h"

namespace Mirror::Reflect {

void SpriteRenderer::cmdRender(const VkCommandBuffer cmd, const VkDescriptorSet vp_set) noexcept {
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindIndexBuffer(cmd, index_buffer.handle, 0, VK_INDEX_TYPE_UINT16);

	for (auto& texture : textures) {
		VkDescriptorSet set_arr[] = { vp_set, texture.set };
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, std::size(set_arr), set_arr, 0, nullptr);

		for (auto& sprite : texture.sprite_queue) {
			vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Sprite), &sprite);
			vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);
		}

		texture.sprite_queue.clear();
	}
}

SpriteRenderer::TextureHandle SpriteRenderer::loadTexture(const std::string_view path, const VkSampler sampler) {
	TextureData texture = TextureData::load(path.data());

	VkExtent3D texture_extent = { static_cast<u32>(texture.width), static_cast<u32>(texture.height), 1 };
	Vk::Image image = Vk::Image::create({
		.extent = texture_extent,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.memory_type = Vk::MemoryType::DeviceLocal,
		.format = VK_FORMAT_R8G8B8A8_SRGB,
	});
	image.writeDeviceLocal(texture.pixels, texture_extent, 4, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkDescriptorSet set = descriptor_pool.allocateSet((usize)0);
	Vk::writeDescriptorSetImage(set, 0, sampler, image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	textures.emplace_back(std::move(image), set);
	return textures.size() - 1;
}

}
