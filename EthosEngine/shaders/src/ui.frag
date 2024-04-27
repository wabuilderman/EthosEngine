#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in mat3 fragTBN;
layout(location = 4) in vec4 fragColor;
layout(location = 5) in vec2 fragTexCoord;
layout(location = 6) in vec2 fragHasTangent;

layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 2) uniform sampler2D diffuseTexSampler;

void main() {
	vec3 baseColor = vec3(1, 1, 1);
	float opacity = 1.0f;

	vec4 diffuseTex = texture(diffuseTexSampler, fragTexCoord);
	baseColor = diffuseTex.xyz * fragColor.xyz;
	opacity = diffuseTex.w * fragColor.w;

	outColor = vec4(baseColor, opacity);
}