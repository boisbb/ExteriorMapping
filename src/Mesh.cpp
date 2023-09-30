#include "Mesh.h"
#include "Buffer.h"
#include "Device.h"
#include "Material.h"
#include "utils/Structs.h"

namespace vke
{

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    : m_vertices(vertices), m_indices(indices), m_modelMatrix{ 1.f },
    m_material(nullptr)
{
}

Mesh::~Mesh()
{
}

void Mesh::afterImportInit(std::shared_ptr<Device> device)
{
    createVertexBuffer(device);
    createIndexBuffer(device);
}

void Mesh::draw(VkCommandBuffer commandBuffer)
{
    VkBuffer vertexBuffers[] = { m_vertexBuffer->getVkBuffer() };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
}

void Mesh::setModelMatrix(glm::mat4 matrix)
{
    m_modelMatrix = matrix;
}

void Mesh::setMaterial(std::shared_ptr<Material> material)
{
    m_material = material;
}

void Mesh::createVertexBuffer(std::shared_ptr<Device> device)
{
    VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

    Buffer stagingBufferO(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBufferO.map();
    stagingBufferO.copyMapped((void*)m_vertices.data(), (size_t)bufferSize);
    stagingBufferO.unmap();

    m_vertexBuffer = std::make_shared<Buffer>(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // void* data;
    device->copyBuffer(stagingBufferO.getVkBuffer(), m_vertexBuffer->getVkBuffer(), m_vertexBuffer->getSize());
}

void Mesh::createIndexBuffer(std::shared_ptr<Device> device)
{
    VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

    Buffer stagingBufferO(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBufferO.map();
    stagingBufferO.copyMapped((void*)m_indices.data(), (size_t)bufferSize);
    stagingBufferO.unmap();
    
    m_indexBuffer = std::make_shared<Buffer>(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    device->copyBuffer(stagingBufferO.getVkBuffer(), m_indexBuffer->getVkBuffer(), m_indexBuffer->getSize());
}

}