#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Space
{
    mat4 view;
    mat4 proj;
};

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outTangent;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() 
{	
    outPos = inPos;
    outUV = inUV;
    outNormal = inNormal;
    outTangent = inTangent;

	gl_Position = proj * view * vec4(inPos, 1.0f);
}