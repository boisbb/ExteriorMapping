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
    mat3 tbn;
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
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec2 inUv;

layout(location = 0) out int outInstanceId;
layout(location = 1) out VsOut vsOut;

void main() 
{
    outInstanceId = gl_InstanceIndex;

    mat4 model = vssbo.objects[gl_InstanceIndex].model;

    gl_Position = ubo.proj * ubo.view * model * vec4(inPosition, 1.0);

    vsOut.fragPosition = vec3(model * vec4(inPosition, 1.0));
    vsOut.fragColor = inColor;
    vsOut.normal = transpose(inverse(mat3(model))) * inNormal;
    vsOut.uv = inUv;

    vec3 t = normalize(vec3(model * vec4(inTangent, 0.0f)));
    vec3 b = normalize(vec3(model * vec4(inBitangent, 0.0f)));
    vec3 n = normalize(vec3(model * vec4(inNormal, 0.0f)));
    vsOut.tbn = mat3(t, b, n);
}