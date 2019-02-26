#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform Space
{
    mat4 view;
    mat4 proj;
};

layout(set = 1, binding = 0) uniform Tranformation
{
    mat4 transformation;
};

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec3 outUV;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() 
{	
    outUV = inPos;
    outUV.y = -outUV.y;

	vec4 mvpPos = proj * view * transformation * vec4(inPos, 1.0f);
    gl_Position = mvpPos.xyww;
}