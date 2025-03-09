#version 460
#extension GL_KHR_vulkan_glsl : enable

layout (set = 0, binding = 0) uniform u {
	mat4 viewMat;
	mat4 projMat;
} vpU;

layout (push_constant) uniform Push {
	mat4 modelMat;
	mat2 texCoords;
} push;

vec2[4] vertices = vec2[](
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),
	vec2(0.0, 1.0)
);

layout (location = 0) out vec2 outTexCoords;

void main() {
	gl_Position = vpU.projMat * vpU.viewMat * push.modelMat * vec4(vertices[gl_VertexIndex] - 0.5, 0.0, 1.0);
	outTexCoords = push.texCoords[0] * vertices[(gl_VertexIndex + 2) % 4] + push.texCoords[1] * vertices[gl_VertexIndex];
}

