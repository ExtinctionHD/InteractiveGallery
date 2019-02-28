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

layout(set = 1, binding = 1) uniform sampler2D dayTexture;
layout(set = 1, binding = 2) uniform sampler2D nightTexture;
layout(set = 1, binding = 3) uniform sampler2D normalTexture;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec4 outColor;

vec3 getBumpedNormal(vec3 N, vec3 T, vec2 uv)
{
	N = normalize(N);

	// texture u vector in world space
	T = normalize(T);
	T = normalize(T - dot(T, N) * N);

	// texture v vector in world space
	vec3 B = cross(T, N);

	// N from texture
	vec3 bumMapNormal = texture(normalTexture, uv).xyz;
	bumMapNormal = 2.0f * bumMapNormal - vec3(1.0f, 1.0f, 1.0f);

	// N from texture in world space
	vec3 resultNormal;
	mat3 tbn = mat3(T, B, N);
	resultNormal = tbn * bumMapNormal;
	resultNormal = normalize(resultNormal);

	return resultNormal;
}

void main() 
{

	vec3 L = normalize(-direction);
	vec3 V = normalize(cameraPos - inPos);
	vec3 N = getBumpedNormal(inNormal, inTangent, inUV);

	float diffuseFactor = max(dot(N, L), 0.0f);

	vec3 result = (directedIntensity * diffuseFactor + ambientIntensity) * texture(dayTexture, inUV).rgb;
	result += pow(1.0f - diffuseFactor, transitionFactor) * texture(nightTexture, inUV).rgb;

    outColor = vec4(result, 1.0f);
}