#pragma once

#include "Device.h"

#include <vector>
#include <memory>

namespace vke
{

class Model;
class Vertex;
class Buffer;
class DescriptorPool;
class DescriptorSetLayout;
class DescriptorSet;

class Scene
{
public:
    Scene();
    ~Scene();

    void setModels(const std::shared_ptr<Device>& device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
        std::shared_ptr<DescriptorPool> descriptorPool, std::vector<std::shared_ptr<Model>> models,
        const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices);
    std::vector<std::shared_ptr<Model>>& getModels();

    void dispatch(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);
    void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame);
    void bindBuffers();

    // TODO: Just for testing now.
    void setLightPos(const glm::vec3& lightPos);
    glm::vec3 getLightPos() const;

private:
    void createVertexBuffer(const std::shared_ptr<Device>& device, const std::vector<Vertex>& vertices);
    void createIndexBuffer(const std::shared_ptr<Device>& device, const std::vector<uint32_t> indices);
    void createIndirectDrawBuffer(const std::shared_ptr<Device>& device);
    void createDescriptorResources(const std::shared_ptr<Device>& device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
        std::shared_ptr<DescriptorPool> descriptorPool);

    std::vector<std::shared_ptr<Model>> m_models;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    std::shared_ptr<Buffer> m_vertexBuffer;
    std::shared_ptr<Buffer> m_indexBuffer;
    std::shared_ptr<Buffer> m_indirectDrawBuffer;

    std::vector<std::unique_ptr<Buffer>> m_indirectDrawBuffers;

    std::vector<std::shared_ptr<DescriptorSet>> m_computeDescriptorSets;

    // TODO: Just for testing now.
    glm::vec3 m_lightPos = { 0, 20, 0 };

    uint32_t m_drawCount;
};

}