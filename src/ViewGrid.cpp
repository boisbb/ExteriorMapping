/**
 * @file ViewGrid.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "ViewGrid.h"

#include "utils/Constants.h"

namespace vke
{

ViewGrid::ViewGrid(std::shared_ptr<Device> device, const glm::vec2 &resolution, const utils::Config &config,
    std::shared_ptr<DescriptorSetLayout> setLayout, std::shared_ptr<DescriptorPool> setPool,
    std::shared_ptr<Model> cameraCube)
    : m_device(device), m_setLayout(setLayout), m_setPool(setPool), m_config(config), m_resolution(resolution),
    m_cameraCube(cameraCube), m_fov(90.f), m_byStep(false), m_gridMatrix(1.f), m_position(0.f), m_step(0.f),
    m_viewDir(0, 0, -1), m_prevViewDir(0, 0, -1), m_byInGridPos(false)
{
    initializeViews();
}

ViewGrid::~ViewGrid()
{
}

void ViewGrid::destroyVkResources()
{
    for (auto& view : m_views)
        view->destroyVkResources();
}

void ViewGrid::viewCalculateEye(std::shared_ptr<View> view)
{
    glm::vec3 gridPos = m_viewGridPos[view];

    glm::vec3 worldPos = m_gridMatrix * glm::vec4(gridPos, 1.f);

    view->setCameraEye(worldPos);}

void ViewGrid::reconstructMatrices()
{
    calculateGridMatrix();

    for (auto& view : m_views)
    {
        if (m_byStep || m_byInGridPos)
            viewCalculateEye(view);
        
        view->getCamera()->reconstructMatrices(m_gridMatrix);
    }
}

void ViewGrid::addColumn()
{
    if (m_views.size() + m_gridSize.y > MAX_VIEWS)
    {
        std::cout << "Error: Maximum number of views exceeded." << std::endl;
        return;
    }

    if (!m_byStep && !m_byInGridPos)
        return;

    if (m_viewRowColumns.size() == 0)
        return;
    
    int newViewWidth = static_cast<float>(m_resolution.x) / static_cast<float>(m_gridSize.x + 1);
    int viewHeight = static_cast<float>(m_resolution.y) / static_cast<float>(m_gridSize.y);

    int newViewWidthOffset = 0;
    int viewHeightOffset = 0;

    int viewId = 0;

    glm::vec2 gridStart(0.f);

    if (m_byStep)
    {
        gridStart = glm::vec2(
            -(((m_config.gridSize.x - 1) * m_step.x) / 2),
            ((m_config.gridSize.y - 1) * m_step.y) / 2
        );
    }

    for (int i = 0; i < m_viewRowColumns.size(); i++)
    {
        viewHeightOffset = viewHeight * i;
        for (int j = 0; j < m_viewRowColumns[i]; j++)
        {
            newViewWidthOffset = j * newViewWidth;

            m_views[viewId]->setResolution(glm::vec2(newViewWidth, viewHeight));
            m_views[viewId]->setViewportStart(glm::vec2(newViewWidthOffset, viewHeightOffset));

            viewId++;
        }

        newViewWidthOffset = m_viewRowColumns[i] * newViewWidth;

        std::shared_ptr<View> view = std::make_shared<View>(glm::vec2(newViewWidth, viewHeight), glm::vec2(newViewWidthOffset, viewHeightOffset),
        m_device, m_setLayout, m_setPool);
        view->setDebugCameraGeometry(m_cameraCube);
        view->getCamera()->setFov(m_fov);
        view->getCamera()->setViewDir(m_prevViewDir);

        if (m_byStep)
        {
            glm::vec3 gridPos = glm::vec3(gridStart, 0.f) + glm::vec3(m_viewRowColumns[i], -i, 0.f) * 
                glm::vec3(m_step, 0.f);

            m_viewGridPos[view] = gridPos;
            viewCalculateEye(view);
        }
        else if (m_byInGridPos)
        {
            m_viewGridPos[view] = glm::vec3(0.f);
            viewCalculateEye(view);
        }

        m_views.insert(m_views.begin() + viewId, view);
        viewId++;
        m_viewRowColumns[i] += 1;
    }

    m_gridSize.x += 1;
}

void ViewGrid::removeColumn(int currentFrame, bool resourcesOnly)
{
    if (m_gridSize.x == 1)
        return;

    if (!m_byStep && !m_byInGridPos)
        return;

    if (m_viewRowColumns.size() == 0)
        return;
    
    int newViewWidth = static_cast<float>(m_resolution.x) / static_cast<float>(m_gridSize.x - 1);
    int viewHeight = static_cast<float>(m_resolution.y) / static_cast<float>(m_gridSize.y);

    int newViewWidthOffset = 0;
    int viewHeightOffset = 0;

    int viewId = 0;

    for (int i = 0; i < m_viewRowColumns.size(); i++)
    {
        if (resourcesOnly)
        {
            viewId = (m_gridSize.y * i) + m_gridSize.x - 1;
            m_views[viewId]->destroyVkResources(currentFrame);
            continue;
        }

        viewHeightOffset = viewHeight * i;
        for (int j = 0; j < m_viewRowColumns[i] - 1; j++)
        {
            newViewWidthOffset = j * newViewWidth;

            m_views[viewId]->setResolution(glm::vec2(newViewWidth, viewHeight));
            m_views[viewId]->setViewportStart(glm::vec2(newViewWidthOffset, viewHeightOffset));

            viewId++;
        }

        m_views[viewId]->destroyVkResources(currentFrame);

        m_views.erase(std::next(m_views.begin(), viewId));
        m_viewRowColumns[i] -= 1;
    }

    if (!resourcesOnly)
        m_gridSize.x -= 1;
}

void ViewGrid::addRow()
{
    if (m_views.size() + m_gridSize.x > MAX_VIEWS)
    {
        std::cout << "Error: Maximum number of views exceeded." << std::endl;
        return;
    }

    if (!m_byStep && !m_byInGridPos)
        return;

    int rowsCount = m_viewRowColumns.size();
    int newViewHeight = static_cast<float>(m_resolution.y) / static_cast<float>(rowsCount + 1);
    int newViewHeightOffset = newViewHeight * rowsCount;
    int columnsCount = m_viewRowColumns[m_viewRowColumns.size() - 1];
    int viewWidth = static_cast<float>(m_resolution.x) / static_cast<float>(columnsCount);

    resizeViewsHeight(newViewHeight);

    glm::vec2 gridStart(0.f);
    if (m_byStep)
    {
        gridStart = glm::vec2(
            -(((m_config.gridSize.x - 1) * m_step.x) / 2),
            ((m_config.gridSize.y - 1) * m_step.y) / 2
        );
    }

    for (int i = 0; i < columnsCount; i++)
    {
        int newViewWidthOffset = viewWidth * i;

        std::shared_ptr<View> view = std::make_shared<View>(glm::vec2(viewWidth, newViewHeight),
            glm::vec2(newViewWidthOffset, newViewHeightOffset), m_device, m_setLayout, m_setPool);
        
        view->setDebugCameraGeometry(m_cameraCube);
        view->getCamera()->setFov(m_fov);
        view->getCamera()->setViewDir(m_prevViewDir);
        m_views.push_back(view);

        if (m_byStep)
        {
            glm::vec3 gridPos = glm::vec3(gridStart, 0.f) + glm::vec3(i, -rowsCount, 0.f) * 
                glm::vec3(m_step, 0.f);

            m_viewGridPos[view] = gridPos;
            viewCalculateEye(view);
        }
        else if (m_byInGridPos)
        {
            m_viewGridPos[view] = glm::vec3(0.f);
            viewCalculateEye(view);
        }
    }

    if (m_byInGridPos || m_byStep)
    {
        m_gridSize.y += 1;
    }

    m_viewRowColumns.push_back(columnsCount);
}

void ViewGrid::removeRow(int currentFrame, bool resourcesOnly)
{
    if (m_gridSize.y == 1)
        return;

    if (!m_byStep && !m_byInGridPos)
        return;

    int columnsCount = m_viewRowColumns[m_viewRowColumns.size() - 1];

    int firstRowViewId = m_views.size() - columnsCount;

    for (int i = firstRowViewId; i < m_views.size(); i++)
    {
        m_views[i]->destroyVkResources(currentFrame);
    }

    if (resourcesOnly)
        return;


    m_views.erase(std::next(m_views.begin(), firstRowViewId), std::next(m_views.begin(), m_views.size()));
    m_viewRowColumns.erase(std::next(m_viewRowColumns.begin(), m_viewRowColumns.size() - 1));

    m_gridSize.y -= 1;

    int rowsCount = m_viewRowColumns.size();
    int newViewHeight = static_cast<float>(m_resolution.y) / static_cast<float>(rowsCount);

    resizeViewsHeight(newViewHeight);
}

std::vector<std::shared_ptr<View>> ViewGrid::getViews() const
{
    return m_views;
}

void ViewGrid::getInputInfo(glm::vec3 &position, glm::vec3 &viewDir, float& speed,
    float& sensitivity)
{
    position = m_position;
    viewDir = m_viewDir;
    speed = m_speed;
    sensitivity = m_sensitivity;
}

bool ViewGrid::getByStep() const
{
    return m_byStep;
}

bool ViewGrid::getByInGridPos() const
{
    return m_byInGridPos;
}

glm::vec3 ViewGrid::getViewGridPos(std::shared_ptr<View> view)
{   
    return m_viewGridPos[view];
}

void ViewGrid::setInputInfo(const glm::vec3 &position, const glm::vec3 &viewDir,
    const float &speed)
{
    m_position = position;
    m_viewDir = viewDir;
    m_speed = speed;
}

void ViewGrid::setViewGridPos(std::shared_ptr<View> view, const glm::vec3& gridPos)
{
    m_viewGridPos[view] = gridPos;
}

void ViewGrid::setFov(float fov)
{
    m_fov = fov;
    
    for (auto& view : m_views)
    {
        view->getCamera()->setFov(fov);
    }
}

glm::vec2 ViewGrid::getResolution() const
{
    return m_resolution;
}

glm::mat4 ViewGrid::getMatrix() const
{
    return m_gridMatrix;
}

std::vector<uint32_t> ViewGrid::getViewRowsColumns() const
{
    return m_viewRowColumns;
}

glm::vec3 ViewGrid::getPos() const
{
    return m_position;
}

glm::vec3 ViewGrid::getViewDir() const
{
    return m_viewDir;
}

glm::vec2 ViewGrid::getGridSize() const
{
    return m_gridSize;
}

float ViewGrid::getFov() const
{
    return m_fov;
}

void ViewGrid::initializeViews()
{
    if (m_config.byStep)
    {
        m_byStep = m_config.byStep;
        initializeByStep();
    }
    else
    {
        if (m_config.byInGridPos)
            initializeByInGridPos();
        else
            initializeByGrid();
    }
}

void ViewGrid::initializeByInGridPos()
{
    m_byInGridPos = m_config.byInGridPos;
    m_byStep = false;
    m_position = m_config.location;
    m_viewDir = m_config.viewDir;
    m_gridSize = m_config.gridSize;
    m_fov = m_config.gridFov;

    calculateGridMatrix();

    glm::vec2 viewResolution = m_resolution / glm::vec2(m_config.gridSize);

    for (int y = 0; y < m_config.gridSize.y; y++)
    {
        m_viewRowColumns.push_back(m_config.gridSize.x);
        for (int x = 0; x < m_config.gridSize.x; x++)
        {
            int viewId = (y * m_gridSize.x) + x;

            vke::utils::Config::View configView = m_config.views[viewId];

            std::shared_ptr<View> view = std::make_shared<View>(viewResolution, viewResolution * glm::vec2(x, y), m_device, 
                m_setLayout, m_setPool);
            view->setDebugCameraGeometry(m_cameraCube);
            view->getCamera()->setFov(m_fov);
            view->getCamera()->setViewDir(configView.viewDir);
            m_views.push_back(view);

            m_viewGridPos[view] = configView.cameraPos;

            viewCalculateEye(view);
        }
    }
}

void ViewGrid::initializeByStep()
{
    m_position = m_config.location;
    m_viewDir = m_config.viewDir;
    m_step = m_config.step;
    m_gridSize = m_config.gridSize;
    m_fov = m_config.gridFov;

    calculateGridMatrix();

    glm::vec2 start = glm::vec2(
        -(((m_config.gridSize.x - 1) * m_config.step.x) / 2),
        ((m_config.gridSize.y - 1) * m_config.step.y) / 2
    );

    glm::vec2 viewResolution = m_resolution / glm::vec2(m_config.gridSize);

    for (int y = 0; y < m_config.gridSize.y; y++)
    {
        m_viewRowColumns.push_back(m_config.gridSize.x);
        for (int x = 0; x < m_config.gridSize.x; x++)
        {
            glm::vec3 gridPos = glm::vec3(start, 0.f) + glm::vec3(x, -y, 0.f) * glm::vec3(m_config.step, 0.f);

            std::shared_ptr<View> view = std::make_shared<View>(viewResolution, viewResolution * glm::vec2(x, y), m_device, 
                m_setLayout, m_setPool);
            view->setDebugCameraGeometry(m_cameraCube);
            view->getCamera()->setFov(m_fov);
            view->getCamera()->setViewDir(m_prevViewDir);
            m_views.push_back(view);

            m_viewGridPos[view] = gridPos;
            viewCalculateEye(view);
        }
    }
}

void ViewGrid::initializeByGrid()
{
    glm::vec3 viewDir = glm::normalize(glm::vec3(-1,0,0));
    m_fov = m_config.gridFov;

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
        m_views.back()->getCamera()->setFov(m_config.gridFov);
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
    
    m_views.insert(m_views.begin() + rowViewStartId + rowViewsCount, view);
    m_viewRowColumns[rowId] += 1;
}

void ViewGrid::resizeViewsHeight(int newViewHeight)
{
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
}

void ViewGrid::calculateGridMatrix()
{
    glm::vec3 org = glm::normalize(m_prevViewDir);
    glm::vec3 target = glm::normalize(m_viewDir);

    float angle = glm::acos(glm::dot(org, target));
    glm::vec3 axis = glm::normalize(glm::cross(org, target));

    m_gridMatrix = glm::translate(glm::mat4(1.f), m_position);
    if (m_prevViewDir != m_viewDir)
        m_gridMatrix = glm::rotate(m_gridMatrix, angle, axis);
}

}