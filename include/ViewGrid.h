#pragma once

#include "glm_include_unified.h"

#include "Device.h"
#include "Camera.h"
#include "View.h"
#include "Model.h"
#include "descriptors/SetLayout.h"
#include "utils/Config.h"

namespace vke
{

class ViewGrid
{
public:
    ViewGrid(std::shared_ptr<Device> device, const glm::vec2& resolution, const utils::Config &config,
        std::shared_ptr<DescriptorSetLayout> setLayout, std::shared_ptr<DescriptorPool> setPool,
        std::shared_ptr<Model> cameraCube);
    ~ViewGrid();

    void viewCalculateEye(std::shared_ptr<View> view);
    void reconstructMatrices();

    std::vector<std::shared_ptr<View>> getViews() const;
    void getInputInfo(glm::vec3& position, glm::vec3& viewDir, float& speed, 
        float& sensitivity);
    void setInputInfo(const glm::vec3& position, const glm::vec3& viewDir, const float& speed);
    glm::vec2 getResolution() const;

private:
    void initializeViews();
    void initializeByStep();
    void initializeByGrid();
    void addViewRow();
    void addViewColumn(int rowId, int rowViewStartId);
    void calculateGridMatrix();
    
    glm::vec2 m_resolution;
    float m_fov;
    bool m_byStep;
    glm::mat4 m_gridMatrix;
    glm::vec3 m_position;
    glm::vec3 m_prevViewDir;
    glm::vec3 m_viewDir;
    glm::vec2 m_step;
    float m_speed = 0.05f;
    float m_sensitivity = 100.f;

    std::shared_ptr<Device> m_device;
    std::shared_ptr<DescriptorSetLayout> m_setLayout;
    std::shared_ptr<DescriptorPool> m_setPool;
    std::shared_ptr<Model> m_cameraCube;

    std::vector<std::shared_ptr<View>> m_views;
    std::vector<uint32_t> m_viewRowColumns;
    std::map<std::shared_ptr<View>, glm::vec2> m_viewGridPos;

    utils::Config m_config;
};

}