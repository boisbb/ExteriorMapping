/**
 * @file ViewGrid.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * 
 */

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
    /**
     * @brief Construct a new View Grid object.
     * 
     * @param device Device
     * @param resolution Resolution of the whole grid.
     * @param config Config parsed from the config file.
     * @param setLayout Descriptor set layout for the views.
     * @param setPool Descriptor set pool for the views.
     * @param cameraCube Camera cube model for visualizing the cameras.
     */
    ViewGrid(std::shared_ptr<Device> device, const glm::vec2& resolution, const utils::Config &config,
        std::shared_ptr<DescriptorSetLayout> setLayout, std::shared_ptr<DescriptorPool> setPool,
        std::shared_ptr<Model> cameraCube);
    ~ViewGrid();

    void destroyVkResources();

    /**
     * @brief Calculates the eye position of the view in grid.
     * 
     * @param view 
     */
    void viewCalculateEye(std::shared_ptr<View> view);

    /**
     * @brief Reconstructs matrices for each views.
     * 
     */
    void reconstructMatrices();

    void addColumn();

    void removeColumn(int currentFrame, bool resourcesOnly = false);

    void addRow();

    void removeRow(int currentFrame, bool resourcesOnly = false);

    // Getters
    std::vector<std::shared_ptr<View>> getViews() const;
    void getInputInfo(glm::vec3& position, glm::vec3& viewDir, float& speed, 
        float& sensitivity);
    bool getByStep() const;
    bool getByInGridPos() const;
    glm::vec3 getViewGridPos(std::shared_ptr<View> view);
    glm::vec2 getResolution() const;
    glm::mat4 getMatrix() const;
    std::vector<uint32_t> getViewRowsColumns() const;
    glm::vec3 getPos() const;
    glm::vec3 getViewDir() const;
    glm::vec2 getGridSize() const;
    float getFov() const;

    // Setters
    void setInputInfo(const glm::vec3& position, const glm::vec3& viewDir, const float& speed);
    void setViewGridPos(std::shared_ptr<View> view, const glm::vec3& gridPos);
    void setFov(float fov);

private:
    // Create and init methods
    void initializeViews();
    void initializeByInGridPos();
    void initializeByStep();
    void initializeByGrid();
    void addViewRow();
    void addViewColumn(int rowId, int rowViewStartId);
    void resizeViewsHeight(int newViewHeight);

    /**
     * @brief Calculates the "view" matrix for the grid.
     * 
     */
    void calculateGridMatrix();
    
    glm::vec2 m_resolution;
    float m_fov;
    bool m_byStep;
    bool m_byInGridPos;
    glm::vec2 m_gridSize;
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
    std::map<std::shared_ptr<View>, glm::vec3> m_viewGridPos;

    utils::Config m_config;
};

}