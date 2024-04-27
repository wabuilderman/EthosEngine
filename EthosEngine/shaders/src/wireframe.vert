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

layout(set = 0, binding = 8) readonly buffer BonesStruct {
	uint count;
	mat4 matrices[];
} bones;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec2 inTexCoord;
layout(location = 5) in vec4 inBoneIndices;
layout(location = 6) in vec4 inBoneWeights;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragCamPos;

layout(location = 4) out struct {
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
} tangentSpace;

void main() {
	// Calculate vertex position in view-space
	vec4 modelPos = vec4(inPosition, 1.0f);
	int index0 = int(inBoneIndices.x);
	int index1 = int(inBoneIndices.y);
	int index2 = int(inBoneIndices.z);
	int index3 = int(inBoneIndices.w);

	vec4 weights = normalize(inBoneWeights);

	vec3 displace = vec3(0);
	if(inBoneIndices.x >= 0) {
		displace += (bones.matrices[index0] * modelPos - modelPos).xyz * weights.x;
		if(inBoneIndices.y >= 0) {
			displace += (bones.matrices[index1] * modelPos - modelPos).xyz * weights.y;
			if(inBoneIndices.z >= 0) {
				displace += (bones.matrices[index2] * modelPos - modelPos).xyz * weights.z;
				if(inBoneIndices.w >= 0) {
					displace += (bones.matrices[index3] * modelPos - modelPos).xyz * weights.w;
				}
			}
		}
		modelPos = vec4(inPosition + displace, 1);
	}
	vec4 worldPos = ubo.model * modelPos; 
	
    gl_Position = ubo.proj * ubo.view * worldPos;

	tangentSpace.tangent = normalize(vec3(ubo.norm * vec4(inTangent, 0)));
	tangentSpace.normal = normalize(vec3(ubo.norm * vec4(inNormal, 0)));

	// Ensure Tangent is perpendicular to normal
	tangentSpace.tangent = normalize(tangentSpace.tangent-dot(tangentSpace.normal,tangentSpace.tangent)*tangentSpace.normal);
	
	// Form BiTangent from normal and tangent
	tangentSpace.bitangent = cross(tangentSpace.normal, tangentSpace.tangent);

	fragCamPos = ubo.camPos;
	fragPos = worldPos.xyz;
    fragColor = normalize(inColor.xyz * ubo.color.xyz);
    fragTexCoord = inTexCoord;
}