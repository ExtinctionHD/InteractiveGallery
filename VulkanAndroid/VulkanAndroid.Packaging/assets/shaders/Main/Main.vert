#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outNormal;

out gl_PerVertex{
    vec4 gl_Position;
};

void main() 
{	
    outPos = inPos;
    outNormal = inNormal;

	gl_Position = vec4(inPos, 1.0f);
}