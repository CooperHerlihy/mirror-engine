#version 460
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;

layout (set = 0, binding = 0) uniform VpUniform {
	mat4 view;
	mat4 projection;
} u_vp;

layout (push_constant) uniform PushConstant {
	mat4 model;
} push;

layout (location = 0) out vec3 frag_position;
layout (location = 1) out vec3 frag_normal;
layout (location = 2) out vec2 frag_uv;

void main() {
	gl_Position = u_vp.projection * u_vp.view * push.model * vec4(in_position, 1.0);

	frag_position = in_position;
	frag_normal = in_normal;
	frag_uv = in_uv;
}
