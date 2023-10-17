#version 450

// layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(set = 1, binding = 1) uniform sampler2D samplers[];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec3 color = vec3(0.0f, 0.0f, 0.0f);

    // outColor = vec4(uv, 0.0f, 1.0f);

    outColor = texture(samplers[0], uv);
}