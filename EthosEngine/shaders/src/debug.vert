#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 model;
    mat4 view;
    mat4 proj;
	mat4 norm;
	vec4 color;
	vec3 camPos;
	vec2 uvCoord;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec2 inTexCoord;
layout(location = 5) in vec4 inBoneIndices;
//layout(location = 6) in vec4 inBoneWeights;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out mat3 fragTBN;
layout(location = 4) out vec4 fragColor;
layout(location = 5) out vec2 fragTexCoord;
layout(location = 6) out vec2 fragHasTangent;


void main() {
	// Calculate vertex position in view-space
	vec4 worldPos = ubo.model * vec4(inPosition, 1.0f);
    gl_Position = ubo.proj * ubo.view * worldPos;

	if(inTangent.length() == 0) {
		fragHasTangent.x = 0;
	} else {
		fragHasTangent.x = 2;
	}

	vec3 T = normalize(vec3(ubo.norm * vec4(inTangent,   0.0)));
	vec3 N = normalize(vec3(ubo.norm * vec4(inNormal,    0.0)));
	vec3 B = cross(N, T);
	fragTBN = mat3(T, B, N);

	fragPos = worldPos.xyz;
    fragColor = inColor.xyzw * ubo.color.xyzw;
    fragTexCoord = inTexCoord * ubo.uvCoord;
}