#version 460
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec2 inTexCoords;

layout (set = 1, binding = 0) uniform sampler2D uSampler;

layout (location = 0) out vec4 outColor;

void main() {
	outColor = texture(uSampler, inTexCoords);
}

