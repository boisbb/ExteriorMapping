/**
 * @file Mesh.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

// vke
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
#include "utils/Math.h"


namespace vke
{

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, MeshInfo info)
    : m_modelMatrix{1.f},
    m_material(nullptr), m_info(info),
    m_vertices(vertices), m_indices(indices),
    m_bbCenter(0.f), m_bbRadius(0.f)   
{
}

Mesh::~Mesh()
{
}

void Mesh::afterImportInit(std::shared_ptr<Device> device,
    std::shared_ptr<Renderer> renderer)
{
    if (m_material->hasTexture())
    {
        handleTexture(device, renderer);
    }

    if (m_material->hasBumpTexture())
    {
        handleBumpTexture(device, renderer);
    }
}

VkDrawIndexedIndirectCommand Mesh::createIndirectDrawCommand(const uint32_t& drawId, uint32_t& instanceId)
{
    VkDrawIndexedIndirectCommand command{};
    command.firstIndex = m_info.firstIndex;
    command.firstInstance = instanceId;
    command.indexCount = m_info.indexCount;
    command.instanceCount = 1;
    command.vertexOffset = m_info.vertexOffset;

    m_drawId = drawId;

    return command;
}

void Mesh::updateDescriptorData(std::vector<MeshShaderDataVertex>& vertexShaderData,
    std::vector<MeshShaderDataFragment>& fragmentShaderData, glm::mat4 modelMatrix)
{
    vertexShaderData.push_back(MeshShaderDataVertex());
    vertexShaderData.back().model = m_modelMatrix;

    fragmentShaderData.push_back(MeshShaderDataFragment());
    if (m_material->hasTexture())
        fragmentShaderData.back().multiple.y = m_material->getTextureId();
    else
        fragmentShaderData.back().multiple.y = RET_ID_NOT_FOUND;

    if (m_material->hasBumpTexture())
        fragmentShaderData.back().multiple.z = m_material->getBumpTextureId();
    else
        fragmentShaderData.back().multiple.z = RET_ID_NOT_FOUND;

    fragmentShaderData.back().diffuseColor = glm::vec4(m_material->getDiffuseColor(), 1.f);
    fragmentShaderData.back().multiple.x = m_material->getOpacity();

}

void Mesh::updateComputeDescriptorData(std::vector<MeshShaderDataCompute> &computeShaderData)
{
    computeShaderData.push_back(MeshShaderDataCompute());
    computeShaderData.back().boundingSphere = glm::vec4(
        m_bbCenter.x,
        m_bbCenter.y,
        m_bbCenter.z,
        m_bbRadius
    );
}

void Mesh::setModelMatrix(const glm::mat4& matrix)
{
    m_modelMatrix = matrix;

    glm::vec3 scale = utils::getScaleFromMatrix(m_modelMatrix);

    float maxScale = std::max(scale.x, std::max(scale.y, scale.z));

    m_bbCenter = glm::vec3(m_modelMatrix * glm::vec4(m_bbCenter, 1.f));
    m_bbRadius *= maxScale;
}

void Mesh::setTransform(const glm::mat4& matrix)
{
    m_modelMatrix = matrix * m_modelMatrix;

    glm::vec3 scale = utils::getScaleFromMatrix(m_modelMatrix);

    float maxScale = std::max(scale.x, std::max(scale.y, scale.z));

    m_bbCenter = glm::vec3(m_modelMatrix * glm::vec4(m_bbCenter, 1.f));
    m_bbRadius *= maxScale;
}

void Mesh::setMaterial(std::shared_ptr<Material> material)
{
    m_material = material;
}

void Mesh::setBbProperties(const glm::vec3& center, float radius)
{
    m_bbCenter = center;
    m_bbRadius = radius;
}

std::shared_ptr<Material> Mesh::getMaterial() const
{
    return m_material;
}

glm::vec3 Mesh::getBbCenter() const
{
    return m_bbCenter;
}

float Mesh::getBbRadius() const
{
    return m_bbRadius;
}

uint32_t Mesh::getDrawId() const
{
    return m_drawId;
}

void Mesh::handleTexture(std::shared_ptr<Device> device, std::shared_ptr<Renderer> renderer)
{
    std::string textureFile = m_material->getTextureFile();

    int textureId = renderer->getTextureId(textureFile);

    if (textureId == RET_ID_NOT_FOUND)
    {
        int width, height, channels;
        unsigned char* pixels = utils::loadImage(textureFile, width, height, channels);

        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
        std::shared_ptr<Texture> newTexture;
        if (channels == 3)
        {
            std::vector<unsigned char> newPixels = utils::threeChannelsToFour(pixels, width, height);
            channels = 4;
            newTexture = std::make_shared<Texture>(device, newPixels.data(), glm::vec2(width, height),
                channels, format);
            // std::cout << "transfering" << std::endl;
        }
        else if (channels == 4)
        {
            newTexture = std::make_shared<Texture>(device, pixels,
                glm::vec2(width, height), channels, format);
        }
        else
            throw std::runtime_error("Error: weird number of channels.");

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
        VkFormat format = VK_FORMAT_R8_UNORM;

        std::shared_ptr<Texture> newTexture;

        if (channels != 1)
        {
            std::vector<unsigned char> newPixels = utils::threeChannelsToOne(pixels, width, height);
            channels = 1;
            newTexture = std::make_shared<Texture>(device, newPixels.data(), glm::vec2(width, height),
                channels, format);
        }
        else {
            newTexture = std::make_shared<Texture>(device, pixels, glm::vec2(width, height),
                channels, format);
        }

        bumpId = renderer->addBumpTexture(newTexture, bumpFile);
    }

    m_material->setBumpTextureId(bumpId);
}

}