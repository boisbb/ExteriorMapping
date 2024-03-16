/**
 * @file Mesh.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

// vulkan
#include <vulkan/vulkan.h>

// std
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>

// glm
#include "glm_include_unified.h"

// vke
#include "Texture.h"

namespace vke
{

class DescriptorSetLayout;
class DescriptorPool;

struct Vertex;
class Buffer;
class Device;
class Material;
class Renderer;
class MeshShaderDataVertex;

}

namespace vke
{

class Mesh
{
public:
    struct MeshInfo
    {
        uint32_t indexCount;
        uint32_t firstIndex;
        uint32_t vertexOffset;
    };

    /**
     * @brief Construct a new Mesh object.
     * 
     * @param vertices Vertices of the mesh.
     * @param indices Indices of the mesh.
     * @param info Info about the mesh for indirect rendering.
     */
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, MeshInfo info);
    ~Mesh();

    /**
     * @brief Initialization of the descriptor resources.
     * 
     * @param device 
     * @param renderer 
     */
    void afterImportInit(std::shared_ptr<Device> device,
        std::shared_ptr<Renderer> renderer);
    
    /**
     * @brief Create a indirect draw command for the mesh.
     * 
     * @param drawId 
     * @param instanceId 
     * @return VkDrawIndexedIndirectCommand 
     */
    VkDrawIndexedIndirectCommand createIndirectDrawCommand(const uint32_t& drawId, uint32_t& instanceId);

    /**
     * @brief Update the descriptor data of the mesh.
     * 
     * @param vertexShaderData 
     * @param fragmentShaderData 
     * @param modelMatrix 
     */
    void updateDescriptorData(std::vector<MeshShaderDataVertex>& vertexShaderData,
        std::vector<MeshShaderDataFragment>& fragmentShaderData, glm::mat4 modelMatrix);
    void updateComputeDescriptorData(std::vector<MeshShaderDataCompute>& computeShaderData);

    // Setters
    void setModelMatrix(const glm::mat4& matrix);
    void setTransform(const glm::mat4& matrix);
    void setMaterial(std::shared_ptr<Material> material);
    void setBbProperties(const glm::vec3& center, float radius);

    // Getters
    std::shared_ptr<Material> getMaterial() const;
    glm::vec3 getBbCenter() const;
    float getBbRadius() const;
    uint32_t getDrawId() const;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
private:
    /**
     * @brief Create and assign texture.
     * 
     * @param device 
     * @param renderer 
     */
    void handleTexture(std::shared_ptr<Device> device, std::shared_ptr<Renderer> renderer);

    /**
     * @brief Create and assign bump texture.
     * 
     * @param device 
     * @param renderer 
     */
    void handleBumpTexture(std::shared_ptr<Device> device, std::shared_ptr<Renderer> renderer);

    MeshInfo m_info;

    glm::vec3 m_bbCenter;
    float m_bbRadius;

    glm::mat4 m_modelMatrix;
    
    std::shared_ptr<Material> m_material;

    uint32_t m_drawId;
};

}