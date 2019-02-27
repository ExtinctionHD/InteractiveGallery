#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D hdrTexture;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

const float EXPOSURE = 1.0f;

void main() 
{
    vec3 hdrColor = texture(hdrTexture, inUV).rgb;

	// tone mapping
    vec3 result = vec3(1.0f) - exp(-hdrColor * EXPOSURE);

	// gamma correction
    // result = pow(result, vec3(1.0 / 2.2f));

    outColor = vec4(result, 1.0f);
}