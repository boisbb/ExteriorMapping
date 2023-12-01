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

class View
{
public:
    View(const glm::vec2& resolution, const glm::vec2& viewportStart, const glm::vec2& scissorStart,
        std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
        std::shared_ptr<DescriptorPool> descriptorPool);
    ~View();

    glm::vec2 getResolution() const;
    glm::vec2 getViewportStart() const;
    glm::vec2 getScissorStart() const;
    std::shared_ptr<Camera> getCamera() const;
    std::shared_ptr<DescriptorSet> getViewDescriptorSet(int currentFrame);

    void setResolution(const glm::vec2& resolution);
    void setViewportStart(const glm::vec2& viewPortStart);
    void setScissorStart(const glm::vec2& scissorStart);
    void setCamera(std::shared_ptr<Camera> camera);

    void updateDescriptorData(int currentFrame);
    void updateComputeDescriptorData(int currentFrame, const std::shared_ptr<Scene>& scene, bool frustumCull);

private:
    void createDescriptorResources(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
    std::shared_ptr<DescriptorPool> descriptorPool);

    glm::vec2 m_resolution;
    glm::vec2 m_viewportStart;
    glm::vec2 m_scissorStart;

    std::shared_ptr<Camera> m_camera;

    std::vector<std::unique_ptr<Buffer>> m_vubos;
    std::vector<std::unique_ptr<Buffer>> m_cubos;

    std::vector<std::shared_ptr<DescriptorSet>> m_viewDescriptorSets;
};

}
