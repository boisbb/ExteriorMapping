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
{
    init();
}

void Application::run()
{
    draw();
}

void Application::init()
{
    m_window = std::make_shared<Window>(WIDTH, HEIGHT);
    m_device = std::make_shared<Device>(m_window);
    m_renderer = std::make_shared<Renderer>(m_device, m_window, "../res/shaders/vert.spv",
        "../res/shaders/frag.spv", "../res/shaders/comp.spv", "../res/shaders/quad_vert.spv",
        "../res/shaders/quad_frag.spv");
    
    m_scene = std::make_shared<Scene>();


    glm::vec3 lightPos = { 0.f, 10.f, 0.f };
    m_scene->setLightPos(lightPos);

    createModels();

    m_scene->setModels(m_device, m_renderer->getSceneComputeDescriptorSetLayout(),
        m_renderer->getSceneComputeDescriptorPool(), m_models, m_vertices, m_indices);

    addViewRow();
    addViewRow();
    
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

        m_renderer->computePass(m_scene, m_views);
        
        uint32_t imageIndex = m_renderer->prepareFrame(m_scene, nullptr, m_window, resizeViews);
        m_renderer->beginCommandBuffer();
        m_renderer->beginRenderPass(m_renderer->getOffscreenRenderPass(), m_renderer->getOffscreenFramebuffer(), windowResolution);
        m_renderer->renderPass(m_scene, m_views);
        m_renderer->endRenderPass();

        m_renderer->beginRenderPass(m_renderer->getQuadRenderPass(), m_renderer->getQuadFramebuffer(imageIndex), windowResolution);
        m_renderer->quadRenderPass(windowResolution);
        renderImgui(lastFps);
        m_renderer->endRenderPass();

        m_renderer->endCommandBuffer();

        m_renderer->submitFrame();
        m_renderer->presentFrame(imageIndex, m_window, nullptr, resizeViews);

        if (resizeViews)
        {
            resizeAllViews();
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
        if (ImGui::Button("-"))
        {
            removeViewRow();
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
        for (auto& info : viewColInfos)
        {
            if (info.remove)
            {
                removeViewColumn(info.rowId, info.viewId + addViewId);
                addViewId -= 1;
            }
            else
            {
                addViewColumn(info.rowId, info.viewId + addViewId);
                addViewId += 1;
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

void Application::createModels()
{
    //std::shared_ptr<Model> negus = vke::utils::importModel("../res/models/negusPlane/negusPlane.obj",
    //    m_vertices, m_indices);
    //negus->afterImportInit(m_device, m_renderer);
    //
    //std::shared_ptr<Model> pepe = vke::utils::importModel("../res/models/pepePlane/pepePlane.obj",
    //    m_vertices, m_indices);
    //pepe->afterImportInit(m_device, m_renderer);

    std::shared_ptr<Model> porsche = vke::utils::importModel("../res/models/porsche/porsche.obj",
        m_vertices, m_indices);
    porsche->afterImportInit(m_device, m_renderer);
    
    std::shared_ptr<Model> sponza = vke::utils::importModel("../res/models/dabrovic_sponza/sponza.obj",
        m_vertices, m_indices);
    sponza->afterImportInit(m_device, m_renderer);

    m_cameraCubeId = m_models.size();

    std::shared_ptr<Model> cameraCube = vke::utils::importModel("../res/models/basicCube/cube.obj",
        m_vertices, m_indices);
    cameraCube->afterImportInit(m_device, m_renderer);

#if DRAW_LIGHT
    m_light->afterImportInit(m_device, m_renderer);
    glm::mat4 lightMatrix = m_light->getModelMatrix();
    lightMatrix = glm::translate(lightMatrix, lightPos);
    lightMatrix = glm::scale(lightMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
    m_light->setModelMatrix(lightMatrix);
#endif

    m_models.push_back(sponza);
    m_models.push_back(porsche);
    m_models.push_back(cameraCube);
}

}