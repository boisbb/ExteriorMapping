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
    : m_modelMatrix{ 1.f },
    m_material(nullptr), m_info(info),
    m_vertices(vertices), m_indices(indices)
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

    if (m_material->hasBumpTexture())
        fragmentShaderData.back().bumpId = m_material->getBumpTextureId();
    else
        fragmentShaderData.back().bumpId = RET_ID_NOT_FOUND;

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

std::shared_ptr<Material> Mesh::getMaterial() const
{
    return m_material;
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
            std::cout << "not transferring" << std::endl;
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