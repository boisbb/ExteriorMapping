#include "Scene.h"
#include "Model.h"
#include "Mesh.h"
#include "Buffer.h"
#include "descriptors/SetLayout.h"
#include "descriptors/Set.h"
#include "descriptors/Pool.h"
#include "utils/Structs.h"

namespace vke
{

Scene::Scene()
    : m_drawCount(0),
    m_indirectDrawBuffers(MAX_FRAMES_IN_FLIGHT),
    m_computeDescriptorSets(MAX_FRAMES_IN_FLIGHT)
{
}

Scene::~Scene()
{
}

void Scene::setModels(const std::shared_ptr<Device>& device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
    std::shared_ptr<DescriptorPool> descriptorPool, std::vector<std::shared_ptr<Model>> models,
    const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices)
{
    m_models = models;

    createVertexBuffer(device, vertices);
    createIndexBuffer(device, indices);
    createIndirectDrawBuffer(device);
    // createDescriptorResources(device, descriptorSetLayout, descriptorPool);
}

std::vector<std::shared_ptr<Model>>& Scene::getModels()
{
    return m_models;
}

void Scene::dispatch(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame)
{
    // VkDescriptorSet computeSet = m_computeDescriptorSets[currentFrame]->getDescriptorSet();
    // vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &computeSet, 0, nullptr);

    vkCmdDispatch(commandBuffer, 1, 1, 1);
}

void Scene::draw(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
    VkBuffer vertexBuffers[] = { m_vertexBuffer->getVkBuffer() };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexedIndirect(commandBuffer, m_indirectDrawBuffer->getVkBuffer(), 0, m_drawCount,
        sizeof(VkDrawIndexedIndirectCommand));

    // uint32_t instanceStart = 0;
    // for (auto& model : m_models)
    //     model->draw(commandBuffer, instanceStart);

    // if (instanceStart != 107)
    //     std::cout << instanceStart << std::endl;
}

void Scene::setLightPos(const glm::vec3& lightPos)
{
    m_lightPos = lightPos;
}

glm::vec3 Scene::getLightPos() const
{
    return m_lightPos;
}

void Scene::createVertexBuffer(const std::shared_ptr<Device>& device,
    const std::vector<Vertex>& vertices)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    Buffer stagingBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.map();
    stagingBuffer.copyMapped((void*)vertices.data(), (size_t)bufferSize);
    stagingBuffer.unmap();

    m_vertexBuffer = std::make_shared<Buffer>(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    device->copyBuffer(stagingBuffer.getVkBuffer(), m_vertexBuffer->getVkBuffer(), m_vertexBuffer->getSize());
}

void Scene::createIndexBuffer(const std::shared_ptr<Device>& device,
    const std::vector<uint32_t> indices)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    Buffer stagingBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.map();
    stagingBuffer.copyMapped((void*)indices.data(), (size_t)bufferSize);
    stagingBuffer.unmap();
    
    m_indexBuffer = std::make_shared<Buffer>(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    device->copyBuffer(stagingBuffer.getVkBuffer(), m_indexBuffer->getVkBuffer(), m_indexBuffer->getSize());
}

void Scene::createIndirectDrawBuffer(const std::shared_ptr<Device>& device)
{
    std::vector<VkDrawIndexedIndirectCommand> commands;

    uint32_t instanceId = 0;
    for (auto& model : m_models)
    {
        model->createIndirectDrawCommands(commands, instanceId);
    }

    m_drawCount = instanceId;

    VkDeviceSize bufferSize = sizeof(VkDrawIndexedIndirectCommand) * commands.size();

    Buffer stagingBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.map();
    stagingBuffer.copyMapped((void*)commands.data(), (size_t)bufferSize);
    stagingBuffer.unmap();

    m_indirectDrawBuffer = std::make_shared<Buffer>(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_indirectDrawBuffer->map();

    device->copyBuffer(stagingBuffer.getVkBuffer(), m_indirectDrawBuffer->getVkBuffer(), m_indirectDrawBuffer->getSize());
}

void Scene::createDescriptorResources(const std::shared_ptr<Device>& device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
    std::shared_ptr<DescriptorPool> descriptorPool)
{
    // for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    // {
    //     m_indirectDrawBuffers[i] = std::make_unique<Buffer>(device, sizeof(VkDrawIndexedIndirectCommand) * MAX_SBOS,
    //         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    //         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    //     m_indirectDrawBuffers[i]->map();
// 
    //     m_indirectDrawBuffers[i]->copyMapped(m_indirectDrawBuffer->getMapped(), m_indirectDrawBuffer->getSize());
// 
    //     m_computeDescriptorSets[i] = std::make_shared<DescriptorSet>(device, descriptorSetLayout, descriptorPool);
// 
    //     std::vector<VkDescriptorBufferInfo> bufferInfos = {
    //         m_indirectDrawBuffers[i]->getInfo()
    //     };
// 
    //     std::vector<uint32_t> bufferBinding = {
    //         0
    //     };
// 
    //     m_computeDescriptorSets[i]->addBuffers(bufferBinding, bufferInfos);
    // }
}

}