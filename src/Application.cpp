#include "Application.h"
#include "utils/Import.h"
#include "utils/Callbacks.h"
#include "utils/Constants.h"
#include "utils/Structs.h"
#include "utils/Input.h"
#include "utils/FileHandling.h"

// std
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <set>
#include <limits>
#include <algorithm>
#include <unordered_map>

#include <chrono>

#define DRAW_LIGHT false

using namespace std::chrono;

namespace vke
{

Application::Application()
    : m_showCameraGeometry(false)
{
    init();
}

void Application::run()
{
    draw();
}

void Application::init()
{
    utils::parseConfig("../res/config.json", m_config);

    m_window = std::make_shared<Window>(WIDTH, HEIGHT);
    m_device = std::make_shared<Device>(m_window);
    m_renderer = std::make_shared<Renderer>(m_device, m_window, "../res/shaders/vert.spv",
        "../res/shaders/frag.spv", "../res/shaders/comp.spv", "../res/shaders/quad_vert.spv",
        "../res/shaders/quad_frag.spv", "../res/shaders/raysEval.spv");

    createScene();

    addConfigViews();
    // addViewRow();
    
    m_renderer->initDescriptorResources();
    initImgui();
}

void Application::draw()
{
    bool resizeViews = false;

    double lastTime = glfwGetTime();
    int frames = 0;

    int lastFps = 0;

    glm::vec2 windowResolution = m_window->getResolution();

    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
        windowResolution = m_window->getResolution();

        consumeInput();

        m_renderer->beginComputePass();
        m_renderer->cullComputePass(m_scene, m_views);
        m_renderer->rayEvalComputePass(m_views);
        m_renderer->endComputePass();
        m_renderer->submitCompute();
        
        uint32_t imageIndex = m_renderer->prepareFrame(m_scene, nullptr, m_window, resizeViews);
        m_renderer->beginCommandBuffer();
        m_renderer->beginRenderPass(m_renderer->getOffscreenRenderPass(), m_renderer->getOffscreenFramebuffer(), windowResolution, glm::vec2(WIDTH, HEIGHT));
        m_renderer->renderPass(m_scene, m_views);
        m_renderer->endRenderPass();

        m_renderer->beginRenderPass(m_renderer->getQuadRenderPass(), m_renderer->getQuadFramebuffer(imageIndex), windowResolution, windowResolution);
        m_renderer->quadRenderPass(windowResolution);
        renderImgui(lastFps);
        m_renderer->endRenderPass();

        m_renderer->endCommandBuffer();

        m_renderer->submitFrame();
        m_renderer->presentFrame(imageIndex, m_window, nullptr, resizeViews);

        // Ray Eval compute pass


        // mainCameraTestRays();

        if (resizeViews)
        {
            // resizeAllViews();
            resizeViews = false;
        }

        double currentTime = glfwGetTime();
        frames++;
        if (currentTime - lastTime >= 1.f)
        {
            lastFps = frames;
            frames = 0;

            lastTime = glfwGetTime();
        }
    }

    vkDeviceWaitIdle(m_device->getVkDevice());
}

void Application::consumeInput()
{
    glfwPollEvents();

    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse && !io.WantCaptureKeyboard)
        vke::utils::consumeDeviceInput(m_window->getWindow(), m_views);
}

void Application::initImgui()
{
    auto indices = m_device->findQueueFamilies(m_device->getPhysicalDevice());

    VkDescriptorPoolSize pool_sizes[] =
	{
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2}
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

    VkDescriptorPool imguiPool;
    vkCreateDescriptorPool(m_device->getVkDevice(), &pool_info, nullptr, &imguiPool);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.DisplaySize = ImVec2(WIDTH, HEIGHT);
    io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

    ImGui_ImplGlfw_InitForVulkan(m_window->getWindow(), true);

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_device->getInstance();
    initInfo.PhysicalDevice = m_device->getPhysicalDevice();
    initInfo.Device = m_device->getVkDevice();
    initInfo.QueueFamily = indices.graphicsFamily.value();
    initInfo.Queue = m_device->getPresentQueue();
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = imguiPool;
    initInfo.Allocator = NULL;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = m_renderer->getSwapChain()->getImageCount();
    initInfo.CheckVkResultFn = NULL;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo, m_renderer->getSwapChain()->getRenderPass());

    ImGui::StyleColorsDark();

    // Use any command queue
    VkCommandBuffer command_buffer = m_renderer->getCommandBuffer(0);

    m_device->beginSingleCommands(command_buffer);

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    m_device->endSingleCommands(command_buffer);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Application::renderImgui(int lastFps)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Config");

    std::string fpsStr = std::to_string(lastFps) + "fps";
    ImGui::Text(fpsStr.c_str());

    if (ImGui::CollapsingHeader("Light"))
    {
        glm::vec3 lightPos = m_scene->getLightPos();
        if (ImGui::SliderFloat3("Light Position:", &lightPos.x, -50.f, 50.f))
        {
            m_scene->setLightPos(lightPos);
            m_scene->setLightChanged(true);
            
#if DRAW_LIGHT
            glm::mat4 lightMatrix = glm::translate(glm::mat4(1.f), lightPos);
            lightMatrix = glm::scale(lightMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
            m_light->setModelMatrix(lightMatrix);
#endif
        }
    }

    if (ImGui::CollapsingHeader("Views"))
    {
        ImGui::Indent();

        if (ImGui::Checkbox("Show camera geometry", &m_showCameraGeometry))
        {
            if (m_showCameraGeometry)
            {
                m_scene->addDebugCameraGeometry(m_views);

            }
            else
            {
                m_scene->setRenderDebugGeometryFlag(false);
            }
        }

        if (ImGui::Button("-"))
        {
            removeViewRow();
            m_scene->setReinitializeDebugCameraGeometryFlag(true);
            m_scene->addDebugCameraGeometry(m_views);
        }
        ImGui::SameLine();
        if (ImGui::Button("+"))
        {
            addViewRow();
        }

        std::vector<ViewColInfo> viewColInfos;

        int viewId = 0;
        for (int i = 0; i < m_viewRowColumns.size(); i++)
        {
            std::string rowText = "Row #" + std::to_string(i);

            if (ImGui::CollapsingHeader(rowText.c_str()))
            {
                ImGui::PushID(i);
                ImGui::Indent();

                if (ImGui::Button("-"))
                {
                    ViewColInfo info;
                    info.rowId = i;
                    info.viewId = viewId;
                    info.remove = true;

                    viewColInfos.push_back(info);
                }

                ImGui::SameLine();

                if (ImGui::Button("+"))
                {                    
                    ViewColInfo info;
                    info.rowId = i;
                    info.viewId = viewId;
                    info.remove = false;

                    viewColInfos.push_back(info);
                }

                for (int j = 0; j < m_viewRowColumns[i]; j++)
                {
                    std::string colText = "#" + std::to_string(j);
                    if (ImGui::CollapsingHeader(colText.c_str()))
                    {
                        auto& view = m_views[viewId + j];

                        ImGui::PushID(view.get());
                        ImGui::Indent();

                        bool depthOnly = view->getDepthOnly();

                        if (ImGui::Checkbox("Render only depth", &depthOnly))
                        {
                            view->setDepthOnly(depthOnly);
                        }

                        bool frustumCulling = view->getFrustumCull();

                        if (ImGui::Checkbox("Frustum Culling", &frustumCulling))
                        {
                            view->setFrustumCull(frustumCulling);
                        }

                        VkDrawIndexedIndirectCommand* commands = m_scene->getViewDrawData(view, m_renderer->getCurrentFrame());
                        
                        int renderedModels = 0;
                        for (int k = 0; k < m_scene->getDrawCount(); k++)
                        {
                            renderedModels += commands[k].instanceCount;
                        }

                        std::string rm = "Rendered meshes: " + std::to_string(renderedModels);

                        ImGui::Text(rm.c_str());

                        ImGui::Unindent();
                        ImGui::PopID();
                    }
                }

                ImGui::Unindent();
                ImGui::PopID();
            }

            viewId += m_viewRowColumns[i];
        }

        if (viewColInfos.size() > 1)
        {
            std::sort(viewColInfos.begin(), viewColInfos.end(), [](const ViewColInfo& a, const ViewColInfo& b)
            {
                return a.viewId < b.viewId;
            });
        }

        int addViewId = 0;
        bool remove = false;
        for (auto& info : viewColInfos)
        {
            if (info.remove)
            {
                removeViewColumn(info.rowId, info.viewId + addViewId);
                addViewId -= 1;
                remove = true;
            }
            else
            {
                addViewColumn(info.rowId, info.viewId + addViewId);
                addViewId += 1;
            }
        }

        if (m_scene->getRenderDebugGeometryFlag())
        {
            if (remove)
            {
                m_scene->setReinitializeDebugCameraGeometryFlag(true);
                m_scene->addDebugCameraGeometry(m_views);
            }
        }

        ImGui::Unindent();
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_renderer->getCommandBuffer(m_renderer->getCurrentFrame()));
}

void Application::cleanup()
{
    
}

void Application::addConfigViews()
{
    uint32_t row = 0;
    std::cout << m_config.views.size() << std::endl;

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
        glm::vec3 viewDir = glm::normalize(glm::vec3(-1,0,0));
        m_views.back()->getCamera()->setViewDir(viewDir);
    }
}

void Application::addViewColumn(int rowId, int rowViewStartId)
{
    if (m_viewRowColumns.size() == 0)
        m_viewRowColumns.push_back(0);
    
    VkExtent2D windowExtent = m_window->getExtent();

    int rowViewsCount = m_viewRowColumns[rowId];

    int newViewWidth = static_cast<float>(windowExtent.width) / static_cast<float>(rowViewsCount + 1);
    int newViewHeight = static_cast<float>(windowExtent.height) / static_cast<float>(m_viewRowColumns.size());

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
        m_device, m_renderer->getViewDescriptorSetLayout(), m_renderer->getViewDescriptorPool());
    view->setDebugCameraGeometry(m_cameraCube);

    // m_scene->setReinitializeDebugCameraGeometryFlag(true);
    // if (m_scene->getRenderDebugGeometryFlag())
    // {
    //     m_scene->addDebugCameraGeometry(m_cameraCube, m_views);
    // }
    
    m_views.insert(m_views.begin() + rowViewStartId + rowViewsCount, view);
    m_viewRowColumns[rowId] += 1;
}

void Application::removeViewColumn(int rowId, int rowViewStartId)
{
    if (m_viewRowColumns.size() == 0)
        return;
    
    if (m_viewRowColumns[rowId] == 1)
        return;
    
    VkExtent2D windowExtent = m_window->getExtent();

    int rowViewsCount = m_viewRowColumns[rowId];

    int newViewWidth = static_cast<float>(windowExtent.width) / static_cast<float>(rowViewsCount - 1);
    int newViewHeight = static_cast<float>(windowExtent.height) / static_cast<float>(m_viewRowColumns.size());

    int newViewWidthOffset = 0;
    int newViewHeightOffset = rowId * newViewHeight;

    for (int i = 0; i < rowViewsCount - 1; i++)
    {
        newViewWidthOffset = newViewWidth * i;

        m_views[rowViewStartId + i]->setResolution(glm::vec2(newViewWidth, newViewHeight));
        m_views[rowViewStartId + i]->setViewportStart(glm::vec2(newViewWidthOffset, newViewHeightOffset));
        
    }

    std::shared_ptr<View> toRemoveView = m_views[rowViewStartId + rowViewsCount - 1];
    m_scene->removeView(toRemoveView);

    m_views.erase(m_views.begin() + rowViewStartId + rowViewsCount - 1);
    m_viewRowColumns[rowId] -= 1;
}

void Application::addViewRow()
{
    VkExtent2D windowExtent = m_window->getExtent();

    int rowsCount = m_viewRowColumns.size();

    int newViewHeight = static_cast<float>(windowExtent.height) / static_cast<float>(rowsCount + 1);
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

    std::shared_ptr<View> view = std::make_shared<View>(glm::vec2(windowExtent.width, newViewHeight),
        glm::vec2(0.f, newViewHeightOffset), m_device, m_renderer->getViewDescriptorSetLayout(),
        m_renderer->getViewDescriptorPool());
    view->setDebugCameraGeometry(m_cameraCube);
    
    m_viewRowColumns.push_back(1);
    m_views.push_back(view);
}

void Application::removeViewRow()
{
    if (m_viewRowColumns.size() == 1)
        return;

    VkExtent2D windowExtent = m_window->getExtent();

    int rowsCount = m_viewRowColumns.size();
    int lastRowViewsCount = m_viewRowColumns.back();

    for (int i = 0; i < lastRowViewsCount; i++)
    {
        std::shared_ptr<View> toRemoveView = m_views[m_views.size() - 1];
        m_scene->removeView(toRemoveView);

        m_views.erase(m_views.end() - 1);
    }

    m_viewRowColumns.erase(m_viewRowColumns.end() - 1);

    int newViewHeight = static_cast<float>(windowExtent.height) / static_cast<float>(rowsCount - 1);
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

void Application::resizeAllViews()
{
    glm::vec2 windowExtent = m_window->getResolution();

    int rowsCount = m_viewRowColumns.size();

    int newViewHeight = static_cast<float>(windowExtent.y) / static_cast<float>(rowsCount);
    int newViewHeightOffset = 0;

    int newViewWidth = 0;
    int newViewWidthOffset = 0;

    int viewId = 0;
    for (int i = 0; i < rowsCount; i++)
    {
        newViewHeightOffset = newViewHeight * i;

        int rowViewsCount = m_viewRowColumns[i];
        newViewWidth = static_cast<float>(windowExtent.x) / static_cast<float>(rowViewsCount);

        for (int j = 0; j < rowViewsCount; j++)
        {
            newViewWidthOffset = newViewWidth * j;

            m_views[viewId]->setResolution(glm::vec2(newViewWidth, newViewHeight));
            m_views[viewId]->setViewportStart(glm::vec2(newViewWidthOffset, newViewHeightOffset));
        
            viewId += 1;
        }
    }
}

void Application::createScene()
{
    m_scene = std::make_shared<Scene>();

    m_scene->setLightPos(m_config.lightPos);

    createModels();

    m_scene->setModels(m_device, m_renderer->getSceneComputeDescriptorSetLayout(),
        m_renderer->getSceneComputeDescriptorPool(), m_models, m_vertices, m_indices);

    m_scene->hideModel(m_cameraCube);
}

void Application::createModels()
{
    for (auto& modelPath : m_config.models)
    {
        std::shared_ptr<Model> model = vke::utils::importModel(modelPath, m_vertices,
            m_indices);
        model->afterImportInit(m_device, m_renderer);

        m_models.push_back(model);
    }

    m_cameraCube = vke::utils::importModel(m_config.viewGeometry,
        m_vertices, m_indices);
    m_cameraCube->afterImportInit(m_device, m_renderer);
    m_models.push_back(m_cameraCube);

#if DRAW_LIGHT
    m_light->afterImportInit(m_device, m_renderer);
    glm::mat4 lightMatrix = m_light->getModelMatrix();
    lightMatrix = glm::translate(lightMatrix, lightPos);
    lightMatrix = glm::scale(lightMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
    m_light->setModelMatrix(lightMatrix);
#endif
}

struct IntersectInfo
{
    glm::vec3 coords;
    float t;
    bool in;
    int planeId;
    std::vector<float> inOut;
};

void Application::mainCameraTestRays()
{
    std::shared_ptr<Camera> mainCamera = m_views[0]->getCamera();
    glm::vec2 mainCameraRes = mainCamera->getResolution();

    std::cout << mainCameraRes.x << " " << mainCameraRes.y << std::endl;

    std::map<int, std::map<std::pair<int, int>, std::vector<IntersectInfo>>> intersectsMap;
    
    for (int y = 0; y < mainCameraRes.y; y++)
    {
        for (int x = 0; x < mainCameraRes.x; x++)
        {

            glm::vec2 pixelCenter(x + 0.5f, y + 0.5f);
            glm::vec2 uv = pixelCenter / mainCameraRes;

            glm::vec2 d = uv * 2.f - 1.f;

            glm::vec4 org4f = mainCamera->getViewInverse() * glm::vec4(0.f, 0.f, 0.f, 1.f);
            glm::vec4 target = mainCamera->getProjectionInverse() * glm::vec4(d.x, d.y, 1.f, 1.f);
            glm::vec4 dir4f = mainCamera->getViewInverse() * glm::vec4(glm::normalize(glm::vec3(target.x, target.y, target.z)), 0.f);

            glm::vec3 org(org4f.x, org4f.y, org4f.z);
            glm::vec3 dir(dir4f.x, dir4f.y, dir4f.z);

            //auto vd = mainCamera->getViewDir();

            for (int i = 1; i < m_views.size(); i++)
            {
                std::shared_ptr<Camera> testCamera = m_views[i]->getCamera();
                std::vector<glm::vec4> frustumPlanes = testCamera->getFrustumPlanes();

                std::vector<IntersectInfo> intersects;

                for (int j = 0; j < frustumPlanes.size(); j++)
                {
                    glm::vec4 currentPlane = frustumPlanes[j];

                    glm::vec3 frustumNormal(
                        currentPlane.x,
                        currentPlane.y,
                        currentPlane.z
                    );

                    float frustumDistance = currentPlane.w;

                    std::vector<float> inOut;

                    if (std::abs(glm::dot(frustumNormal, dir)) > 1e-6)
                    {
                        // https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution.html
                        float t = -(glm::dot(frustumNormal, org) + frustumDistance) / glm::dot(frustumNormal, dir);
                        glm::vec3 intersect = org + t * dir;

                        bool isInside = true;

                        if (t < 0)
                            isInside = false;
                        else
                        {
                            for (int k = 0; k < frustumPlanes.size(); k++)
                            {
                                if (k == j)
                                    continue;
                                
                                glm::vec4 checkPlane = frustumPlanes[k];

                                glm::vec3 checkNormal(
                                    checkPlane.x,
                                    checkPlane.y,
                                    checkPlane.z
                                );

                                float checkDistance = checkPlane.w;

                                if (glm::dot(checkNormal, intersect) + checkDistance < 0.f)
                                {
                                    isInside = false;
                                }
                            }
                        }
                        

                        if (isInside)
                        {
                            IntersectInfo intersectInfo = {
                                intersect, 
                                t,
                                isInside,
                                j,
                                inOut
                            };

                            intersects.push_back(intersectInfo);
                        }
                    }
                }

                if (intersects.size() > 0)
                {
                    intersectsMap[i][std::pair<int, int>(x, y)] = intersects;
                }
            }
        }
    }
    
    for (auto& kvV : intersectsMap)
    {
        std::cout << "View: " << kvV.first << std::endl;

        std::array<int, 6> planesForView{0,0,0,0,0,0};
        for (int y = 0; y < mainCameraRes.y; y++)
        {
            for (int x = 0; x < mainCameraRes.x; x++)
            {                    
                int cnt = 0;
                for (auto& intersect : kvV.second[std::pair<int,int>(x, y)])
                {
                    planesForView[intersect.planeId]++;
                }
            }
        }

        for (int i = 0; i < 6; i++)
        {
            std::cout << "    plane: " << i << " intersects: " << planesForView[i] << std::endl;
        }

        std::cout << std::endl;
    }
}

}