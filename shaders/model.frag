#version 460
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;

layout (set = 1, binding = 0) uniform sampler2D u_sampler;

layout (location = 0) out vec4 out_color;

void main() {
	vec4 tex_color = texture(u_sampler, in_uv);

	out_color = tex_color;
}
