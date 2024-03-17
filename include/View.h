/**
 * @file View.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "glm_include_unified.h"

#include "Device.h"
#include "Camera.h"
#include "descriptors/Set.h"
#include "descriptors/SetLayout.h"
#include "descriptors/Pool.h"

#include <memory>

namespace vke
{

class Buffer;
class Scene;
class Model;

class View
{
public:
    /**
     * @brief Construct a new View object.
     * 
     * @param resolution Resolution of the view.
     * @param viewportStart Starting point of the view within the viewport.
     * @param device Device.
     * @param descriptorSetLayout Decriptor set layout for the view. 
     * @param descriptorPool Descriptor pool for the view.
     */
    View(const glm::vec2& resolution, const glm::vec2& viewportStart, std::shared_ptr<Device> device,
        std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, std::shared_ptr<DescriptorPool> descriptorPool);
    ~View();

    void destroyVkResources(int currentFrame = -1);

    // Getters
    glm::vec2 getResolution() const;
    glm::vec2 getViewportStart() const;
    std::shared_ptr<Camera> getCamera() const;
    std::shared_ptr<DescriptorSet> getViewDescriptorSet(int currentFrame);
    bool getFrustumCull() const;
    bool getDepthOnly() const;
    glm::vec2 getNearFar() const;

    // Setters
    void setResolution(const glm::vec2& resolution);
    void setViewportStart(const glm::vec2& viewPortStart);
    void setCamera(std::shared_ptr<Camera> camera);
    void setCameraEye(glm::vec3 eye);
    void setFrustumCull(bool frustumCull);
    void setDepthOnly(bool depthOnly);

    /**
     * @brief Updates the view desriptor data.
     * 
     * @param currentFrame 
     */
    void updateDescriptorData(int currentFrame);

    /**
     * @brief Updates the compute view descriptor data.
     * 
     * @param currentFrame 
     * @param scene 
     */

    /**
     * @brief Update the view descriptor data for the debug camera geometry.
     * 
     * @param currentFrame 
     * @param scene 
     */
    void updateComputeDescriptorData(int currentFrame, const std::shared_ptr<Scene>& scene);
    void updateDescriptorDataRenderDebugCube(std::vector<MeshShaderDataVertex>& vertexShaderData,
        std::vector<MeshShaderDataFragment>& fragmentShaderData);

    // Debug
    void setDebugCameraGeometry(std::shared_ptr<Model> model);
    std::shared_ptr<Model> getDebugCameraModel() const;

private:
    /**
     * @brief Create a Descriptor Resources.
     * 
     * @param device 
     * @param descriptorSetLayout 
     * @param descriptorPool 
     */
    void createDescriptorResources(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
    std::shared_ptr<DescriptorPool> descriptorPool);

    glm::vec2 m_resolution;
    glm::vec2 m_viewportStart;

    std::shared_ptr<Camera> m_camera;

    std::vector<std::unique_ptr<Buffer>> m_vubos;
    std::vector<std::unique_ptr<Buffer>> m_cubos;
    std::vector<std::unique_ptr<Buffer>> m_fubos;

    std::vector<std::shared_ptr<DescriptorSet>> m_viewDescriptorSets;

    bool m_frustumCull;
    bool m_depthOnly;

    // Debug
    std::shared_ptr<Model> m_debugModel;
};

}
