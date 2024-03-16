/**
 * @file Scene.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "Device.h"
#include "utils/Constants.h"

#include <vector>
#include <map>
#include <array>
#include <memory>

namespace vke
{

class Model;
class Vertex;
class Camera;
class View;
class Buffer;
class DescriptorPool;
class DescriptorSetLayout;
class DescriptorSet;

class Scene
{
public:
    /**
     * @brief Construct a new Scene object.
     * 
     */
    Scene();
    ~Scene();

    void destroyVkResources();

    /**
     * @brief Set the Models.
     * 
     * @param device Vulkan device object.
     * @param descriptorSetLayout Descriptor set layout for the model resources.
     * @param descriptorPool Descriptor pool for the model resources.
     * @param models Vector of models.
     * @param vertices Total vertices vector.
     * @param indices Total indices vector.
     */
    void setModels(const std::shared_ptr<Device>& device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
        std::shared_ptr<DescriptorPool> descriptorPool, std::vector<std::shared_ptr<Model>> models,
        const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices);
    void setLightChanged(bool lightChanged);
    void setSceneChanged(bool sceneChanged);

    std::vector<std::shared_ptr<Model>>& getModels();
    uint32_t getDrawCount() const;
    VkDrawIndexedIndirectCommand* getViewDrawData(std::shared_ptr<View> view, int currentFrame);

    bool lightChanged() const;
    bool sceneChanged() const;
    bool viewResourcesExist(std::shared_ptr<View> view);

    /**
     * @brief Dispatch the compute.
     * 
     * @param view 
     * @param commandBuffer 
     * @param pipelineLayout 
     * @param currentFrame 
     */
    void dispatch(std::shared_ptr<View> view, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);

    /**
     * @brief Draw the scene.
     * 
     * @param view 
     * @param commandBuffer 
     * @param currentFrame 
     */
    void draw(std::shared_ptr<View> view, VkCommandBuffer commandBuffer, uint32_t currentFrame);

    /**
     * @brief Create a View Resources.
     * 
     * @param view 
     * @param device 
     * @param descriptorSetLayout 
     * @param descriptorPool 
     */
    void createViewResources(std::shared_ptr<View> view, const std::shared_ptr<Device>& device,
        std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, std::shared_ptr<DescriptorPool> descriptorPool);

    void setLightPos(const glm::vec3& lightPos);
    glm::vec3 getLightPos() const;

    /**
     * @brief Hides the model by setting its indirect command.
     * 
     * @param model 
     */
    void hideModel(std::shared_ptr<Model> model);

    // Debug
    void addDebugCameraGeometry(std::vector<std::shared_ptr<View>> views);
    void setRenderDebugGeometryFlag(bool renderDebugCameraGeometryFlag);
    bool getRenderDebugGeometryFlag() const;
    void setReinitializeDebugCameraGeometryFlag(bool reinitializeDebugCameraGeometry);

private:
    // Create methods
    void createVertexBuffer(const std::shared_ptr<Device>& device, const std::vector<Vertex>& vertices);
    void createIndexBuffer(const std::shared_ptr<Device>& device, const std::vector<uint32_t> indices);
    void createIndirectDrawBuffer(const std::shared_ptr<Device>& device);

    std::vector<std::shared_ptr<Model>> m_models;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    std::shared_ptr<Buffer> m_vertexBuffer;
    std::shared_ptr<Buffer> m_indexBuffer;
    std::shared_ptr<Buffer> m_indirectDrawBuffer;

    std::map<std::shared_ptr<View>, std::array<std::shared_ptr<Buffer>, MAX_FRAMES_IN_FLIGHT>> m_indirectBuffersMap;
    std::map<std::shared_ptr<View>, std::array<std::shared_ptr<DescriptorSet>, MAX_FRAMES_IN_FLIGHT>> m_computeDescriptorsMap;
    std::map<std::shared_ptr<Model>, std::array<int, 2>> m_modelDrawRef;

    bool m_sceneChanged;

    // TODO: Just for testing now.
    glm::vec3 m_lightPos = { 0, 20, 0 };
    bool m_lightChanged;

    uint32_t m_drawCount;

    // Debug
    bool m_renderDebugCameraGeometry;
    bool m_reinitializeDebugCameraGeometry;
    int m_renderDebugViewsDrawCount;
};

}