#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform Lighting
{
    vec3 direction;
	float directedIntensity;
    vec3 cameraPos;
	float transitionPower;
};

layout(set = 1, binding = 0) uniform samplerCube cubeTexture;

layout(location = 0) in vec3 inUV;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = texture(cubeTexture, inUV);
}