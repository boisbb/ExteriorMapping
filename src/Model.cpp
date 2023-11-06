#include "Model.h"
#include "Mesh.h"
#include "Device.h"
#include "Renderer.h"
#include "descriptors/SetLayout.h"
#include "descriptors/Pool.h"
#include "utils/Structs.h"

namespace vke
{

Model::Model()
    : m_modelMatrix{1.f}
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
    std::shared_ptr<Renderer> renderer)
{
    for (auto mesh : m_meshes)
        mesh->afterImportInit(device, renderer);
}

void Model::createIndirectDrawCommands(std::vector<VkDrawIndexedIndirectCommand>& commands,
        uint32_t& instanceId)
{
    for (auto& mesh : m_meshes)
    {
        VkDrawIndexedIndirectCommand command = mesh->createIndirectDrawCommand(instanceId);
        commands.push_back(command);

        instanceId++;
    }
}

void Model::updateDescriptorData(std::vector<MeshShaderDataVertex>& vertexShaderData,
    std::vector<MeshShaderDataFragment>& fragmentShaderData)
{
    for (auto mesh : m_meshes)
    {
        mesh->updateDescriptorData(vertexShaderData, fragmentShaderData, m_modelMatrix);
    }
}

void Model::setModelMatrix(glm::mat4 matrix)
{
    m_modelMatrix = matrix;
}

glm::mat4 Model::getModelMatrix() const
{
    return m_modelMatrix;
}

}