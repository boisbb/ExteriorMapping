/**
 * @file Model.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * 
 */

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
    /**
     * @brief Construct a new Model object
     * 
     */
    Model();
    ~Model();

    // Getters
    std::vector<std::shared_ptr<Mesh>> getMeshes() const;
    size_t getMeshesCount() const;
    size_t getTransparentMeshesCount() const;

    /**
     * @brief Adds mesh into a model.
     * 
     * @param mesh 
     */
    void addMesh(std::shared_ptr<Mesh> mesh);

    /**
     * @brief Initialize descriptor data after initialization.
     * 
     * @param device 
     * @param renderer 
     */
    void afterImportInit(std::shared_ptr<Device> device,
        std::shared_ptr<Renderer> renderer);

    /**
     * @brief Create a Indirect Draw Commands for transparent meshes.
     * 
     * @param commands 
     * @param instanceId 
     */
    void createIndirectDrawCommandsTransparent(std::vector<VkDrawIndexedIndirectCommand>& commands,
        uint32_t& instanceId);

    /**
     * @brief Create a Indirect Draw Commands for regular meshes.
     * 
     * @param commands 
     * @param instanceId 
     */
    void createIndirectDrawCommands(std::vector<VkDrawIndexedIndirectCommand>& commands,
        uint32_t& instanceId);
    
    /**
     * @brief Update descriptor data of the meshes.
     * 
     * @param vertexShaderData 
     * @param fragmentShaderData 
     * @param transparentMeshes 
     */
    void updateDescriptorData(std::vector<MeshShaderDataVertex>& vertexShaderData,
        std::vector<MeshShaderDataFragment>& fragmentShaderData, bool transparentMeshes = false);
    
    /**
     * @brief Update descriptor data for the compute pass.
     * 
     * @param computeShaderData 
     * @param transparentMeshes 
     */
    void updateComputeDescriptorData(std::vector<MeshShaderDataCompute>& computeShaderData, bool transparentMeshes = false);

    void setModelMatrix(const glm::mat4& matrix);
    glm::mat4 getModelMatrix() const;

    void checkMeshesVisible(std::shared_ptr<Camera> camera, VkDrawIndexedIndirectCommand* commands);
private:
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    std::vector<std::shared_ptr<Mesh>> m_transparentMeshes;

    glm::mat4 m_modelMatrix;
};

}