#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragCamPos;

layout(location = 4) in struct {
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
} tangentSpace;

#define POINT_LIGHT 0u
#define SUN_LIGHT 1u

struct LightType {
	vec3 pos;
	vec4 color;
	vec3 dir;
	uint type;
};

layout(set = 1, binding = 1) readonly buffer Buffer {
    uint count;
    LightType lights[];
} lightBuffer;

layout(set = 0, binding = 4) uniform Material {
	vec3 baseColor;
	vec4 emission;

	float metallic;
	float specular;
	float roughness;
	float sheen;
	float IOR;

	vec2 diffuseUVScale;
	vec2 normalUVScale;
	vec2 specularUVScale;
	vec2 ambientUVScale;
	vec2 displaceUVScale;
} material;

layout(set = 0, binding = 2) uniform sampler2D diffuseTexSampler;
layout(set = 0, binding = 3) uniform sampler2D normalTexSampler;
layout(set = 0, binding = 5) uniform sampler2D specularTexSampler;
layout(set = 0, binding = 6) uniform sampler2D ambientTexSampler;
layout(set = 0, binding = 7) uniform sampler2D displaceTexSampler;

layout(location = 0) out vec4 outColor;

float calculateFalloff(float lightDist) {
	return 1.0f / (1.0f + lightDist * lightDist);
}

float calculateIntensity(vec3 lightDir, vec3 normal) {
	return max(dot(normal, lightDir), 0.0);
}

float calculateSpecular(vec3 lightDir, vec3 normal, vec3 camDir, float specPow, float specMul) {
	vec3 reflectDir = reflect(-lightDir, normal); 
	return pow(max(dot(camDir, reflectDir), 0.0), specPow) * specMul;
}


void main() {
	// Calculate camera-direction
	vec3 camDir = normalize(fragCamPos - fragPos);

	// Calculate tangentSpace matrix
	mat3 TBN = mat3(
		normalize(tangentSpace.tangent),
		normalize(tangentSpace.bitangent),
		normalize(tangentSpace.normal)
	);

	vec3 lighting = vec3(0, 0, 0);
	
	// Determine base-color and opacity
	vec3 baseColor = material.baseColor;
	float opacity = 1.0f;
	if(material.diffuseUVScale.x > 0) {
		vec4 diffuseTex = texture(diffuseTexSampler, fragTexCoord * material.diffuseUVScale);
		baseColor = diffuseTex.xyz;
		opacity = diffuseTex.w;
	}

	// Dertmine environmental light
	vec3 ambientLight = vec3(0.05f, 0.05f, 0.05f);
	if(material.ambientUVScale.x > 0) {
		ambientLight = ambientLight * texture(ambientTexSampler, fragTexCoord * material.ambientUVScale).r;
	}
	lighting += ambientLight;

	// Determine final fragment normal
	vec3 norm;
	if(material.normalUVScale.x > 0) {
		vec3 normalTex = texture(normalTexSampler, fragTexCoord * material.normalUVScale).rgb;
		norm = normalize(normalTex * 2.0 - 1.0);
		norm = TBN * norm;
		norm = normalize(tangentSpace.normal + 0.5 * norm);
	} else {
		norm = normalize(tangentSpace.normal);
	}
	//norm = normalize(tangentSpace.normal);

	// Determine local specular intensity
	float specularIntensity = 1.0f;
	if(material.specularUVScale.x > 0) {
		specularIntensity = texture(specularTexSampler, fragTexCoord * material.specularUVScale).r;
	}

	// Illumination by light source
	for(uint i = 0; i < lightBuffer.count; ++i) {
		vec3 lightDir = {0, 0, 0};
		float lightDist = 0;
		float intensity = 0;

		vec3 lightColor = normalize(lightBuffer.lights[i].color.rgb);

		switch(lightBuffer.lights[i].type) {
			case POINT_LIGHT:
				lightDir = normalize(lightBuffer.lights[i].pos - fragPos);
				lightDist = length(lightBuffer.lights[i].pos - fragPos);
				intensity = calculateIntensity(lightDir, norm) * calculateFalloff(lightDist);
				break;

			case SUN_LIGHT:
				lightDir = -normalize(lightBuffer.lights[i].dir);
				intensity = calculateIntensity(lightDir, norm);
				break;
		}

		intensity *= lightBuffer.lights[i].color.w;

		float specular = calculateSpecular(lightDir, norm, camDir, 80.0f, specularIntensity * 4 + 0.1) * intensity;
		float diffuse = intensity;

		lighting += (specular + diffuse) * lightColor;
	}

	// Self-Illumination
	lighting += material.emission.xyz;

	if(length(fragColor) != 0) {
		baseColor *= fragColor;
	}

	vec3 finalColor = baseColor * lighting;
	//vec3 finalColor = texture(displaceTexSampler, texCoord * material.displaceUVScale).rgb;

	//vec3 finalColor = norm;

	outColor = vec4(finalColor, opacity);
}