#version 450

#include "structs.glsl"

layout(set=0, binding=0) uniform PointsUniformBuffer {
    mat4 view;
    mat4 proj;
    vec2 viewImageRes;
    vec2 viewCount;
    vec2 sampledView;
    vec2 pointsRes;
} ubo;

layout(std430, set=0, binding=1) readonly buffer ssbo {
    PointsStorageBuffer objects[];
} vssbo;

layout(set=0, binding=2) uniform sampler2D viewImageSampler;

layout(set=0, binding=3) uniform sampler2D viewDepthSampler;

layout (location = 0) out vec4 outColor;

void main() 
{
    int linearSampledView = int((ubo.sampledView.y * ubo.viewCount.x) + ubo.sampledView.x);

    PointsStorageBuffer viewData = vssbo.objects[linearSampledView];
    vec2 startUv = viewData.resOffset.zw / ubo.viewImageRes;
    vec2 sizeUv = viewData.resOffset.xy / ubo.viewImageRes;

    vec2 vertexPixelCoords = vec2(mod(gl_VertexIndex,ubo.pointsRes.x),
        floor(gl_VertexIndex / ubo.pointsRes.x));
    vec2 vertexPixelUv = vertexPixelCoords / ubo.pointsRes.xy;
    
    vec2 viewsImageUv = startUv + vertexPixelUv * sizeUv;

    float n = viewData.nearFar.x;
    float f = viewData.nearFar.y;
    float z = texture(viewDepthSampler, viewsImageUv).r;

    vec2 clipXy = (vertexPixelUv * 2.0f) - 1.0f;

    vec4 clipPoint = vec4(clipXy, z, 1.0);
    vec4 viewPoint = viewData.invProj * clipPoint;
    viewPoint /= viewPoint.w;

    vec3 worldPoint = (viewData.invView * viewPoint).xyz;

	gl_Position = ubo.proj * ubo.view * vec4(worldPoint, 1.0f);
    gl_PointSize = 2.0;
    outColor = texture(viewImageSampler, viewsImageUv);
}