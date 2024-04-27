#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragCamPos;

layout(location = 4) in struct {
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
} tangentSpace;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(fragColor, 1.0f);
}