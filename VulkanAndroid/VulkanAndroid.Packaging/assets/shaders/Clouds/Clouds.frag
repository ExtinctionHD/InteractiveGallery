#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform Lighting
{
    vec3 direction;
    float directedIntensity;
    vec3 cameraPos;
    float transitionFactor;
    float ambientIntensity;
};

layout(set = 1, binding = 0) uniform sampler2D cloudsTexture;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec4 outColor;

void main() 
{

	vec3 L = normalize(-direction);
	vec3 V = normalize(cameraPos - inPos);
	vec3 N = normalize(inNormal);

	float diffuseFactor = max(dot(N, L), 0.0f);

	float color = texture(cloudsTexture, inUV).r;

	float result = (directedIntensity * diffuseFactor + ambientIntensity) * color;

    outColor = vec4(result, result, result, color);
}