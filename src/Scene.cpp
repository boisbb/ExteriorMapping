#include "Scene.h"
#include "Model.h"
#include "Mesh.h"
#include "Buffer.h"
#include "utils/Structs.h"

namespace vke
{

Scene::Scene()
    : m_drawCount(0)
{
}

Scene::~Scene()
{
}

void Scene::setModels(const std::shared_ptr<Device>& device, std::vector<std::shared_ptr<Model>> models,
    const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices)
{
    m_models = models;

    createVertexBuffer(device, vertices);
    createIndexBuffer(device, indices);
    createIndirectDrawBuffer(device);
}

std::vector<std::shared_ptr<Model>>& Scene::getModels()
{
    return m_models;
}

void Scene::draw(VkCommandBuffer commandBuffer)
{
    VkBuffer vertexBuffers[] = { m_vertexBuffer->getVkBuffer() };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexedIndirect(commandBuffer, m_indirectDrawBuffer->getVkBuffer(), 0, m_drawCount,
        sizeof(VkDrawIndexedIndirectCommand));
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
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    device->copyBuffer(stagingBuffer.getVkBuffer(), m_indirectDrawBuffer->getVkBuffer(), m_indirectDrawBuffer->getSize());
}

}