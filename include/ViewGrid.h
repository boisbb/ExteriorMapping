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