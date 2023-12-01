#include "Model.h"
#include "Mesh.h"
#include "Material.h"
#include "Device.h"
#include "Renderer.h"
#include "Camera.h"
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
    if (mesh->getMaterial()->isTransparent())
        m_transparentMeshes.push_back(mesh);
    else
        m_meshes.push_back(mesh);

}

void Model::afterImportInit(std::shared_ptr<Device> device,
    std::shared_ptr<Renderer> renderer)
{
    for (auto& mesh : m_meshes)
        mesh->afterImportInit(device, renderer);
    
    for (auto& mesh : m_transparentMeshes)
        mesh->afterImportInit(device, renderer);
}

void Model::createIndirectDrawCommandsTransparent(std::vector<VkDrawIndexedIndirectCommand> &commands, 
    uint32_t &instanceId)
{
    uint32_t drawId = commands.size();
    for (auto& mesh : m_transparentMeshes)
    {

        VkDrawIndexedIndirectCommand command = mesh->createIndirectDrawCommand(drawId, instanceId);
        commands.push_back(command);

        instanceId++;
        drawId++;
    }
}

void Model::createIndirectDrawCommands(std::vector<VkDrawIndexedIndirectCommand> &commands,
    uint32_t &instanceId)
{
    uint32_t drawId = commands.size();
    for (auto& mesh : m_meshes)
    {
        VkDrawIndexedIndirectCommand command = mesh->createIndirectDrawCommand(drawId, instanceId);
        commands.push_back(command);

        instanceId++;
    }
}

void Model::updateDescriptorData(std::vector<MeshShaderDataVertex>& vertexShaderData,
    std::vector<MeshShaderDataFragment>& fragmentShaderData)
{
    for (auto& mesh : m_meshes)
        mesh->updateDescriptorData(vertexShaderData, fragmentShaderData, m_modelMatrix);

    for (auto& mesh : m_transparentMeshes)
        mesh->updateDescriptorData(vertexShaderData, fragmentShaderData, m_modelMatrix);

}

void Model::updateComputeDescriptorData(std::vector<MeshShaderDataCompute> &computeShaderData)
{
    for (auto& mesh : m_meshes)
        mesh->updateComputeDescriptorData(computeShaderData);

    for (auto& mesh : m_transparentMeshes)
        mesh->updateComputeDescriptorData(computeShaderData);
}

void Model::setModelMatrix(const glm::mat4& matrix)
{
    m_modelMatrix = matrix;

    for (auto& mesh : m_meshes)
        mesh->setTransform(matrix);
    
    for (auto& mesh : m_transparentMeshes)
        mesh->setTransform(matrix);
}

glm::mat4 Model::getModelMatrix() const
{
    return m_modelMatrix;
}


// Frustum culling cpu test
void Model::checkMeshesVisible(std::shared_ptr<Camera> camera, VkDrawIndexedIndirectCommand *commands)
{
    for (auto& mesh : m_meshes)
    {
        glm::vec3 center = mesh->getBbCenter();
        float radius = mesh->getBbRadius();

        std::vector<glm::vec4> frustumPlanes = camera->getFrustumPlanes();

        for (auto i = 0; i < frustumPlanes.size(); i++)
        {
            if ((frustumPlanes[i].x * center.x) + (frustumPlanes[i].y * center.y) + (frustumPlanes[i].z * center.z) + frustumPlanes[i].w <= -radius)
            {
                uint32_t drawId = mesh->getDrawId();

                commands[drawId].instanceCount = 0;
            }
        }
    }
}

}