#pragma once

#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Texture.h"

namespace vke
{

class DescriptorSetLayout;
class DescriptorPool;

struct Vertex;
class Buffer;
class Device;
class Material;
class Renderer;
class MeshShaderDataVertex;

}

namespace vke
{

class Mesh
{
public:
    struct MeshInfo
    {
        uint32_t indexCount;
        uint32_t firstIndex;
        uint32_t vertexOffset;
    };

    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, MeshInfo info);
    ~Mesh();

    void afterImportInit(std::shared_ptr<Device> device,
        std::shared_ptr<Renderer> renderer);
    VkDrawIndexedIndirectCommand createIndirectDrawCommand(uint32_t& instanceId);
    void updateDescriptorData(std::vector<MeshShaderDataVertex>& vertexShaderData,
        std::vector<MeshShaderDataFragment>& fragmentShaderData, glm::mat4 modelMatrix);

    void setModelMatrix(glm::mat4 matrix);
    void setMaterial(std::shared_ptr<Material> material);

    std::shared_ptr<Material> getMaterial() const;

    // testing
    void draw(VkCommandBuffer commandBuffer, uint32_t& instanceStart);
    void createVertexBuffer(std::shared_ptr<Device> device);
    void createIndexBuffer(std::shared_ptr<Device> device);
    
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    std::shared_ptr<Buffer> m_vertexBuffer;
    std::shared_ptr<Buffer> m_indexBuffer;
private:
    void handleTexture(std::shared_ptr<Device> device, std::shared_ptr<Renderer> renderer);
    void handleBumpTexture(std::shared_ptr<Device> device, std::shared_ptr<Renderer> renderer);

    MeshInfo m_info;
    
    glm::mat4 m_modelMatrix;
    
    std::shared_ptr<Material> m_material;
};

}