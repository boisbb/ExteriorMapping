#pragma once

#include <iostream>
#include <optional>
#include <vector>
#include <array>

#include <vulkan/vulkan.h>

// GLM
#include "glm_include_unified.h"

namespace vke
{

enum class SamplingType { 
    COLOR,
    DEPTH_DIST,
    DEPTH_ANGLE,
    END
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// Regular shader data
struct ViewDataVertex {
    glm::mat4 view;
    glm::mat4 proj;
};

struct UniformDataFragment {
    glm::vec3 lightPos;
    float __padding;
};

struct ViewDataFragment {
    bool depthOnly;
};

struct MeshShaderDataVertex {
    glm::mat4 model;
};

struct MeshShaderDataFragment {
    glm::vec4 diffuseColor;
    // x - opacity, y - colorTex id
    // z - bumpTexId
    glm::vec4 multiple;
    //float opacity;
    //int textureId;
    //int bumpId;
    //float __padding[2];
};

// Cull shader data
struct ViewDataCompute {
    glm::vec4 frustumPlanes[6];
    unsigned int totalMeshes;
    bool frustumCull;
};

struct MeshShaderDataCompute {
    glm::vec4 boundingSphere;
};


// Ray Eval shader data
struct RayEvalUniformBuffer {
    glm::mat4 invView;
    glm::mat4 invProj;
    glm::vec2 res;
    glm::vec2 viewsTotalRes;
    int viewCnt;
    unsigned int samplingType;
    bool testPixel;
    float __padding;
    glm::vec2 testedPixel;
    int numOfRaySamples;
};

struct ViewEvalDataCompute {
    glm::vec4 frustumPlanes[6];
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 invView;
    glm::mat4 invProj;
    glm::vec4 resOffset;
    glm::vec2 nearFar;
    float __padding[2];
};

struct ViewEvalDebugCompute {
    glm::vec4 frustumPlanes[6];
    int numOfIntersections;
    int numOfFoundIntervals;
    glm::vec2 viewRes;
    glm::vec4 pointInWSpace;
    // glm::vec2 t[32];
    // uint ids[128];
    //float __padding[2];
};

struct FrustumHit
{
    float t;
    int viewId;
};

struct RayEvalParams 
{
    bool testPixel;
    glm::vec2 testedPixel;
    int numOfRaySamples;
};

// Quad shaders
struct QuadUniformBuffer
{
    bool m_depthOnly;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec2 uv;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, normal);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, tangent);

        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[4].offset = offsetof(Vertex, bitangent);

        attributeDescriptions[5].binding = 0;
        attributeDescriptions[5].location = 5;
        attributeDescriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[5].offset = offsetof(Vertex, uv);

        return attributeDescriptions;
    }
};

}