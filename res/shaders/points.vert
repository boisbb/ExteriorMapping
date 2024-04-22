#version 450

#include "structs.glsl"

layout(set=0, binding=0) uniform PointsUniformBuffer {
    mat4 view;
    mat4 proj;
    vec2 viewImageRes;
    vec2 viewCount;
    vec2 sampledView;
} ubo;

layout(std430, set=0, binding=1) readonly buffer ssbo {
    PointsStorageBuffer objects[];
} vssbo;

layout(set=0, binding=2) uniform sampler2D viewImageSampler;

layout(set=0, binding=3) uniform sampler2D viewDepthSampler;

layout (location = 0) out vec4 outColor;

void main() 
{
	gl_Position = ubo.proj * ubo.view * vec4(9.0f, 2.0f, 0.0f, 1.0f);
    gl_PointSize = 2.0 + vssbo.objects[0].resOffset.z;
    outColor = texture(viewImageSampler, vec2(0.0));
}