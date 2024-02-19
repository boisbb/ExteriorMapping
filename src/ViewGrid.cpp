#include "ViewGrid.h"

#include "utils/Constants.h"

namespace vke
{

ViewGrid::ViewGrid(std::shared_ptr<Device> device, const glm::vec2 &resolution, const utils::Config &config,
    std::shared_ptr<DescriptorSetLayout> setLayout, std::shared_ptr<DescriptorPool> setPool,
    std::shared_ptr<Model> cameraCube)
    : m_device(device), m_setLayout(setLayout), m_setPool(setPool), m_config(config), m_resolution(resolution),
    m_cameraCube(cameraCube), m_fov(90.f), m_byStep(false)
{
}

ViewGrid::~ViewGrid()
{
}

void ViewGrid::viewCalculateEye(std::shared_ptr<View> view)
{
    glm::vec2 gridPos = m_viewGridPos[view];

    glm::vec3 worldPos = m_gridMatrix * glm::vec4(gridPos, 0.f, 1.f);

    view->setCameraEye(worldPos);
}

void ViewGrid::initializeViews()
{
    if (m_config.byStep)
    {
        initializeByStep();
    }
    else
    {
        initializeByGrid();
    }
}

void ViewGrid::initializeByStep()
{
    glm::vec2 start = glm::vec2(
        -(m_config.gridSize.x * m_config.step.x / 2),
        m_config.gridSize.y * m_config.step.y / 2
    );

    glm::vec2 viewResolution = m_resolution / glm::vec2(m_config.gridSize);

    for (int y = 0; y < m_config.gridSize.y; y++)
    {
        for (int x = 0; x < m_config.gridSize.x; x++)
        {
            glm::vec3 gridPos = glm::vec3(start, 0.f) + glm::vec3(x, y, 0.f) * glm::vec3(m_config.gridSize, 0.f);

            std::shared_ptr<View> view = std::make_shared<View>(viewResolution, viewResolution * glm::vec2(x, y), m_device, 
                m_setLayout, m_setPool);
            view->setDebugCameraGeometry(m_cameraCube);
            view->getCamera()->setFov(m_fov);
            view->getCamera()->setViewDir(m_config.viewDir);
            m_views.push_back(view);

            m_viewGridPos[view] = gridPos;
            viewCalculateEye(view);
            // view->getCamera()->setCameraEye(eyePos);
        }
    }
}

void ViewGrid::initializeByGrid()
{
    glm::vec3 viewDir = glm::normalize(glm::vec3(-1,0,0));
    calculateGridMatrix();

    uint32_t row = 0;
    int currentRowStartId = 0;
    for (int i = 0; i < m_config.views.size(); i++)
    {
        utils::Config::View configView = m_config.views[i];
        
        row = configView.row;

        if (m_viewRowColumns.size() == row)
        {
            currentRowStartId = m_views.size();
            addViewRow();
        }
        else
        {
            addViewColumn(row, currentRowStartId);
        }

        m_views.back()->setCameraEye(configView.cameraPos);
        m_views.back()->getCamera()->setViewDir(viewDir);
    }
}

void ViewGrid::addViewRow()
{
    int rowsCount = m_viewRowColumns.size();

    int newViewHeight = static_cast<float>(m_resolution.y) / static_cast<float>(rowsCount + 1);
    int newViewHeightOffset = 0;
    
    int viewId = 0;
    for (int i = 0; i < m_viewRowColumns.size(); i++)
    {
        newViewHeightOffset = newViewHeight * i;
        
        for (int j = 0; j < m_viewRowColumns[i]; j++)
        {
            auto& view = m_views[viewId];

            glm::vec2 viewOffset =  view->getViewportStart();
            viewOffset.y = newViewHeightOffset;

            glm::vec2 viewResolution = view->getResolution();
            viewResolution.y = newViewHeight;

            view->setViewportStart(viewOffset);
            view->setResolution(viewResolution);

            viewId += 1;
        }
    }

    newViewHeightOffset = newViewHeight * rowsCount;

    std::shared_ptr<View> view = std::make_shared<View>(glm::vec2(m_resolution.x, newViewHeight),
        glm::vec2(0.f, newViewHeightOffset), m_device, m_setLayout,
        m_setPool);
    view->setDebugCameraGeometry(m_cameraCube);
    view->getCamera()->setFov(m_fov);
    
    m_viewRowColumns.push_back(1);
    m_views.push_back(view);
}

void ViewGrid::addViewColumn(int rowId, int rowViewStartId)
{
    if (m_viewRowColumns.size() == 0)
        m_viewRowColumns.push_back(0);
    
    int rowViewsCount = m_viewRowColumns[rowId];

    int newViewWidth = static_cast<float>(m_resolution.x) / static_cast<float>(rowViewsCount + 1);
    int newViewHeight = static_cast<float>(m_resolution.y) / static_cast<float>(m_viewRowColumns.size());

    int newViewWidthOffset = 0;
    int newViewHeightOffset = rowId * newViewHeight;

    for (int i = 0; i < rowViewsCount; i++)
    {
        newViewWidthOffset = newViewWidth * i;

        m_views[rowViewStartId + i]->setResolution(glm::vec2(newViewWidth, newViewHeight));
        m_views[rowViewStartId + i]->setViewportStart(glm::vec2(newViewWidthOffset, newViewHeightOffset));
        
    }

    newViewWidthOffset = newViewWidth * rowViewsCount;

    std::shared_ptr<View> view = std::make_shared<View>(glm::vec2(newViewWidth, newViewHeight), glm::vec2(newViewWidthOffset, newViewHeightOffset),
        m_device, m_setLayout, m_setPool);
    view->setDebugCameraGeometry(m_cameraCube);
    view->getCamera()->setFov(m_fov);

    // m_scene->setReinitializeDebugCameraGeometryFlag(true);
    // if (m_scene->getRenderDebugGeometryFlag())
    // {
    //     m_scene->addDebugCameraGeometry(m_cameraCube, m_views);
    // }
    
    m_views.insert(m_views.begin() + rowViewStartId + rowViewsCount, view);
    m_viewRowColumns[rowId] += 1;
}

void ViewGrid::calculateGridMatrix()
{
    glm::vec3 org = glm::normalize(glm::vec3(0, 0, -1));
    glm::vec3 target = glm::normalize(m_config.viewDir);

    float angle = glm::acos(glm::dot(org, target));
    glm::vec3 axis = glm::normalize(glm::cross(org, target));

    m_gridMatrix = glm::translate(glm::mat4(1.f), m_config.location);
    m_gridMatrix = glm::rotate(m_gridMatrix, angle, axis);
}

}