#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

struct SharedBufferObject {
    mat4 model;
};

layout(std140, binding = 1) readonly buffer storageBuffer {
    SharedBufferObject objects[];
} sbo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 uv;

void main() 
{
    mat4 model = sbo.objects[gl_InstanceIndex].model;
    gl_Position = ubo.proj * ubo.view * model * vec4(inPosition, 1.0);
    fragColor = inColor;
    normal = transpose(inverse(mat3(model))) * inNormal;
    uv = inUv;
}