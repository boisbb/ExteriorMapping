#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec3 color = vec3(0.0f, 0.0f, 0.0f);

    outColor = vec4(normal, 1.0);
}