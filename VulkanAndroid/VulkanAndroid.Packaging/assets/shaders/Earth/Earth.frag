#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform Lighting
{
    vec3 color;
    float ambientStrength;
    vec3 direction;
    float directedStrength;
    vec3 cameraPos;
    float specularPower;
} lighting;

layout(set = 1, binding = 1) uniform sampler2D dayTexture;
layout(set = 1, binding = 2) uniform sampler2D nightTexture;
layout(set = 1, binding = 3) uniform sampler2D normalTexture;
layout(set = 1, binding = 4) uniform sampler2D specularTexture;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec4 outColor;

vec3 getBumpedNormal(vec3 normal, vec3 tangent, vec2 uv)
{
	normal = normalize(normal);

	// texture u vector in world space
	tangent = normalize(tangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);

	// texture v vector in world space
	vec3 bitangent = cross(tangent, normal);

	// normal from texture
	vec3 bumMapNormal = texture(normalTexture, uv).xyz;
	bumMapNormal = 2.0f * bumMapNormal - vec3(1.0f, 1.0f, 1.0f);

	// normal from texture in world space
	vec3 resultNormal;
	mat3 tbn = mat3(tangent, bitangent, normal);
	resultNormal = tbn * bumMapNormal;
	resultNormal = normalize(resultNormal);

	return resultNormal;
}

void main() 
{
	vec3 L = normalize(-lighting.direction);
	vec3 V = normalize(lighting.cameraPos - inPos);
	vec3 N = getBumpedNormal(inNormal, inTangent, inUV);

	vec3 diffuse = max(dot(N, L), 0.0) * lighting.color * lighting.directedStrength;
	vec3 result = (lighting.ambientStrength + diffuse) * texture(dayTexture, inUV).rgb;

    outColor = vec4(result, 1.0f);
}