#include "Model.h"
#include "Mesh.h"
#include "Device.h"
#include "descriptors/SetLayout.h"
#include "descriptors/Pool.h"

namespace vke
{

Model::Model()
{
}

Model::~Model()
{
}

std::vector<std::shared_ptr<Mesh>> Model::getMeshes() const
{
    return m_meshes;
}

void Model::addMesh(std::shared_ptr<Mesh> mesh)
{
    m_meshes.push_back(mesh);
}

void Model::afterImportInit(std::shared_ptr<Device> device,
    std::unordered_map<std::string, std::shared_ptr<Texture>>& textureMap,
    std::shared_ptr<DescriptorSetLayout> setLayout, std::shared_ptr<DescriptorPool> setPool)
{
    for (auto mesh : m_meshes)
        mesh->afterImportInit(device, textureMap, setLayout, setPool);
}

void Model::draw(VkCommandBuffer commandBuffer)
{
    for (auto mesh : m_meshes)
    {
        mesh->draw(commandBuffer);
    }
}

}