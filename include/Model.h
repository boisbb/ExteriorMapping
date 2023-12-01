#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>
#include <unordered_map>

#include "Texture.h"

namespace vke
{

class DescriptorSetLayout;
class DescriptorPool;
class Mesh;
class Device;
class Camera;
class Renderer;
class MeshShaderDataVertex;

class Model
{
public:
    Model();
    ~Model();

    std::vector<std::shared_ptr<Mesh>> getMeshes() const;

    void addMesh(std::shared_ptr<Mesh> mesh);

    void afterImportInit(std::shared_ptr<Device> device,
        std::shared_ptr<Renderer> renderer);

    void createIndirectDrawCommandsTransparent(std::vector<VkDrawIndexedIndirectCommand>& commands,
        uint32_t& instanceId);
    void createIndirectDrawCommands(std::vector<VkDrawIndexedIndirectCommand>& commands,
        uint32_t& instanceId);
    void updateDescriptorData(std::vector<MeshShaderDataVertex>& vertexShaderData,
        std::vector<MeshShaderDataFragment>& fragmentShaderData);
    void updateComputeDescriptorData(std::vector<MeshShaderDataCompute>& computeShaderData);

    void setModelMatrix(const glm::mat4& matrix);
    glm::mat4 getModelMatrix() const;

    void checkMeshesVisible(std::shared_ptr<Camera> camera, VkDrawIndexedIndirectCommand* commands);
private:
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    std::vector<std::shared_ptr<Mesh>> m_transparentMeshes;

    glm::mat4 m_modelMatrix;
};

}