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
    Scene();
    ~Scene();

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

    void dispatch(std::shared_ptr<View> view, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);
    void draw(std::shared_ptr<View> view, VkCommandBuffer commandBuffer, uint32_t currentFrame);
    void createViewResources(std::shared_ptr<View> view, const std::shared_ptr<Device>& device,
        std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, std::shared_ptr<DescriptorPool> descriptorPool);

    // TODO: Just for testing now.
    void setLightPos(const glm::vec3& lightPos);
    glm::vec3 getLightPos() const;

    void checkModelsVisible(std::shared_ptr<Camera> camera, int currentFrame);
    void hideModel(std::shared_ptr<Model> model);
    void removeView(std::shared_ptr<View> view);

    // Debug
    void addDebugCameraGeometry(std::vector<std::shared_ptr<View>> views);
    void setRenderDebugGeometryFlag(bool renderDebugCameraGeometryFlag);
    bool getRenderDebugGeometryFlag() const;
    void setReinitializeDebugCameraGeometryFlag(bool reinitializeDebugCameraGeometry);

private:
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