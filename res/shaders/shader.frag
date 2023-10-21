#version 450

// layout(set = 1, binding = 0) uniform sampler2D texSampler;

struct MeshShaderDataFragment {
    int textureId;
};

layout(std140, binding = 2) readonly buffer storageBufferFrag {
    MeshShaderDataFragment objects[];
} sbof;

layout(set = 1, binding = 0) uniform sampler2D samplers[2];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) flat in int instanceId;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec3 color = vec3(0.0f, 0.0f, 0.0f);

    // outColor = vec4(uv, 0.0f, 1.0f);

    int textureId = instanceId;//sbof.objects[0].textureId;

    outColor = texture(samplers[textureId], uv);
}