#include "Mesh.h"
#include "Buffer.h"
#include "Device.h"
#include "Material.h"
#include "Renderer.h"
#include "descriptors/SetLayout.h"
#include "descriptors/Pool.h"
#include "utils/Structs.h"
#include "utils/FileHandling.h"
#include "utils/Constants.h"

namespace vke
{

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
    MeshInfo info)
    : m_vertices(vertices), m_indices(indices), m_modelMatrix{ 1.f },
    m_material(nullptr), m_info(info)
{
}

Mesh::~Mesh()
{
}

void Mesh::afterImportInit(std::shared_ptr<Device> device,
    std::shared_ptr<Renderer> renderer)
{
    createVertexBuffer(device);
    createIndexBuffer(device);

    if (m_material->hasTexture())
    {
        handleTexture(device, renderer);
    }

    if (m_material->hasBumpTexture())
    {
        handleBumpTexture(device, renderer);
    }
}

void Mesh::draw(VkCommandBuffer commandBuffer, uint32_t& instanceStart)
{
    VkBuffer vertexBuffers[] = { m_vertexBuffer->getVkBuffer() };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, instanceStart);

    instanceStart += 1;
}

VkDrawIndexedIndirectCommand Mesh::createIndirectDrawCommand(uint32_t& instanceId)
{
    VkDrawIndexedIndirectCommand command{};
    command.firstIndex = m_info.firstIndex;
    command.firstInstance = instanceId;
    command.indexCount = m_info.indexCount;
    command.instanceCount = 1;
    command.vertexOffset = m_info.vertexOffset;

    return command;
}

void Mesh::updateDescriptorData(std::vector<MeshShaderDataVertex>& vertexShaderData,
    std::vector<MeshShaderDataFragment>& fragmentShaderData, glm::mat4 modelMatrix)
{
    vertexShaderData.push_back(MeshShaderDataVertex());
    vertexShaderData.back().model = modelMatrix * m_modelMatrix;

    fragmentShaderData.push_back(MeshShaderDataFragment());
    if (m_material->hasTexture())
        fragmentShaderData.back().textureId = m_material->getTextureId();
    else
        fragmentShaderData.back().textureId = RET_ID_NOT_FOUND;
    fragmentShaderData.back().diffuseColor = m_material->getDiffuseColor();
    fragmentShaderData.back().opacity = m_material->getOpacity();
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

void Mesh::handleTexture(std::shared_ptr<Device> device, std::shared_ptr<Renderer> renderer)
{
    std::string textureFile = m_material->getTextureFile();

    int textureId = renderer->getTextureId(textureFile);

    if (textureId == RET_ID_NOT_FOUND)
    {
        int width, height, channels;
        unsigned char* pixels = utils::loadImage(textureFile, width, height, channels);

        VkFormat format;
        if (channels == 3)
            format = VK_FORMAT_R8G8B8_SRGB;
        else if (channels == 4)
            format = VK_FORMAT_R8G8B8A8_SRGB;
        else
            throw std::runtime_error("Error: weird number of channels.");

        std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(device, pixels,
            glm::vec2(width, height), channels, format);

        textureId = renderer->addTexture(newTexture, textureFile);
    }

    m_material->setTextureId(textureId);
}

void Mesh::handleBumpTexture(std::shared_ptr<Device> device, std::shared_ptr<Renderer> renderer)
{
    std::string bumpFile = m_material->getBumpTextureFile();

    int bumpId = renderer->getBumpTextureId(bumpFile);

    if (bumpId == RET_ID_NOT_FOUND)
    {
        int width, height, channels;
        unsigned char* pixels = utils::loadImage(bumpFile, width, height, channels);

        VkFormat format;
        if (channels == 1)
            format = VK_FORMAT_R8_UNORM;
        else
        {
            m_material->setHasBumpTexture(false);
            m_material->setBumpTextureFile("");

            return;
        }

        std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(device, pixels,
            glm::vec2(width, height), channels, format);

        bumpId = renderer->addBumpTexture(newTexture, bumpFile);
    }

    m_material->setBumpTextureId(bumpId);
}

}