#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 1) uniform sampler2D dayTexture;
layout(set = 1, binding = 2) uniform sampler2D nightTexture;
layout(set = 1, binding = 3) uniform sampler2D bumpTexture;
layout(set = 1, binding = 4) uniform sampler2D specularTexture;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = texture(dayTexture, inUV);
}