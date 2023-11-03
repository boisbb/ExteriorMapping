#version 460

struct MeshShaderDataVertex {
    mat4 model;
};

struct VsOut
{
    vec3 fragPosition;
    vec3 fragColor;
    vec3 normal;
    vec2 uv;
}; 

layout(binding = 0) uniform UniformDataVertex {
    mat4 view;
    mat4 proj;
} ubo;

layout(std430, binding = 1) readonly buffer ssbo {
    MeshShaderDataVertex objects[];
} vssbo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUv;

layout(location = 0) out VsOut vsOut;
layout(location = 5) out int outInstanceId;

void main() 
{
    outInstanceId = gl_InstanceIndex;

    mat4 model = vssbo.objects[gl_InstanceIndex].model;

    gl_Position = ubo.proj * ubo.view * model * vec4(inPosition, 1.0);

    vsOut.fragPosition = vec3(model * vec4(inPosition, 1.0));
    vsOut.fragColor = inColor;
    vsOut.normal = transpose(inverse(mat3(model))) * inNormal;
    vsOut.uv = inUv;
}