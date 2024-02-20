#include "ViewGrid.h"

#include "utils/Constants.h"

namespace vke
{

ViewGrid::ViewGrid(std::shared_ptr<Device> device, const glm::vec2 &resolution, const utils::Config &config,
    std::shared_ptr<DescriptorSetLayout> setLayout, std::shared_ptr<DescriptorPool> setPool,
    std::shared_ptr<Model> cameraCube)
    : m_device(device), m_setLayout(setLayout), m_setPool(setPool), m_config(config), m_resolution(resolution),
    m_cameraCube(cameraCube), m_fov(90.f), m_byStep(false), m_gridMatrix(1.f), m_position(0.f), m_step(0.f),
    m_viewDir(0, 0, -1), m_prevViewDir(0, 0, -1)
{
    initializeViews();
}

ViewGrid::~ViewGrid()
{
}

void ViewGrid::viewCalculateEye(std::shared_ptr<View> view)
{
    glm::vec3 gridPos = m_viewGridPos[view];

    glm::vec3 worldPos = m_gridMatrix * glm::vec4(gridPos, 1.f);

    // std::cout << worldPos.x << " " << worldPos.y << " " << worldPos.z << std::endl;

    view->setCameraEye(worldPos);}

void ViewGrid::reconstructMatrices()
{
    calculateGridMatrix();

    for (auto& view : m_views)
    {
        if (m_byStep)
            viewCalculateEye(view);
        
        view->getCamera()->reconstructMatrices(m_gridMatrix);
    }
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

glm::vec2 ViewGrid::getResolution() const
{
    return m_resolution;
}

glm::mat4 ViewGrid::getMatrix() const
{
    return m_gridMatrix;
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
        initializeByGrid();
    }
}

void ViewGrid::initializeByStep()
{
    m_position = m_config.location;
    m_viewDir = m_config.viewDir;
    m_step = m_config.step;

    calculateGridMatrix();

    glm::vec2 start = glm::vec2(
        -(((m_config.gridSize.x - 1) * m_config.step.x) / 2),
        ((m_config.gridSize.y - 1) * m_config.step.y) / 2
    );

    glm::vec2 viewResolution = m_resolution / glm::vec2(m_config.gridSize);

    for (int y = 0; y < m_config.gridSize.y; y++)
    {
        for (int x = 0; x < m_config.gridSize.x; x++)
        {
            glm::vec3 gridPos = glm::vec3(start, 0.f) + glm::vec3(x, -y, 0.f) * glm::vec3(m_config.step, 0.f);

            std::cout << gridPos.x << " " << gridPos.y << " " << gridPos.z << std::endl;

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
    
    m_views.insert(m_views.begin() + rowViewStartId + rowViewsCount, view);
    m_viewRowColumns[rowId] += 1;
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