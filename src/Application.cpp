/**
 * @file Application.cpp
 * @author Boris Burkalo (xburka00)
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

// vke
#include "Application.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "utils/Import.h"
#include "utils/Callbacks.h"
#include "utils/Constants.h"
#include "utils/Structs.h"
#include "utils/Input.h"
#include "utils/FileHandling.h"
#include "utils/VulkanHelpers.h"

// std
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <set>
#include <limits>
#include <algorithm>
#include <unordered_map>
#include <chrono>
#include <sstream>

// usings
using namespace std::chrono;

// Sampling type strings for ImGui.
std::array<std::string, 3> samplingTypeStrings = {
    "color",
    "depth with distance",
    "depth with angle distance"
};

namespace vke
{

Application::Application(std::string configFile)
    : m_showCameraGeometry(false),
    m_renderFromViews(false),
    m_changeOffscreenTarget(MAX_FRAMES_IN_FLIGHT),
    m_samplingType(SamplingType::COLOR),
    m_testedPixel(0.f, 0.f),
    m_intervalCounter(m_interval)
{
    init(configFile);
}

Application::~Application()
{
    vkDeviceWaitIdle(m_device->getVkDevice());
    m_renderer->destroyVkResources();

    m_scene->destroyVkResources();
    
    m_novelViewGrid->destroyVkResources();
    m_viewGrid->destroyVkResources();

    m_viewMatrixScreenshotImage->destroyVkResources();
    m_novelViewScreenshotImage->destroyVkResources();
    m_actualViewScreenshotImage->destroyVkResources();

    m_window->destroyVkResources(m_device->getInstance());
    m_secondaryWindow->destroyVkResources(m_device->getInstance());

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    vkDestroyDescriptorPool(m_device->getVkDevice(), m_imguiPool, nullptr);

    m_device->destroyVkResources();
}

void Application::run()
{
    draw();

    vke::utils::saveConfig(std::string(CONFIG_FILES_LOC) + "last.json", m_config, m_novelViewGrid, m_viewGrid);
}

void Application::init(std::string configFile)
{
    utils::parseConfig(configFile, m_config);

    m_window = std::make_shared<Window>(WINDOW_WIDTH, WINDOW_HEIGHT);

    m_device = std::make_shared<Device>(m_window);

    RendererInitParams params{"offscreen.vert.spv",
        "offscreen.frag.spv", "cull.comp.spv", "quad.vert.spv",
        "quad.frag.spv", "novelView.comp.spv", "generatePoints.comp.spv"};
    m_renderer = std::make_shared<Renderer>(m_device, m_window, params);
    m_renderer->setNovelViewSamplingType(m_samplingType);

    m_secondaryWindow = std::make_shared<Window>(WINDOW_WIDTH, WINDOW_HEIGHT, false);
    m_secondaryWindow->createWindowSurface(m_device->getInstance());
    glfwSetWindowCloseCallback(m_secondaryWindow->getWindow(), secondaryWindowCloseCallback);

    m_renderer->addSecondaryWindow(m_secondaryWindow);

    createScene();
    createMainView();

    VkExtent2D offscreenRes = m_renderer->getOffscreenFramebuffer()->getResolution();
    m_viewGrid = std::make_shared<ViewGrid>(m_device, glm::vec2(offscreenRes.width, offscreenRes.height), m_config, 
        m_renderer->getViewDescriptorSetLayout(), m_renderer->getViewDescriptorPool(), m_cameraCube);
    
    m_viewsFov = m_config.gridFov;

    m_renderer->initDescriptorResources();
    
    initImgui();
    renderViewMatrix();
    vkDeviceWaitIdle(m_device->getVkDevice());

    m_viewMatrixScreenshotImage = std::make_shared<Image>(m_device, glm::vec2(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT), VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_viewMatrixScreenshotImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);

    m_novelViewScreenshotImage = std::make_shared<Image>(m_device, glm::vec2(NOVEL_VIEW_WIDTH, NOVEL_VIEW_HEIGHT), VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_novelViewScreenshotImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);

    m_actualViewScreenshotImage = std::make_shared<Image>(m_device, glm::vec2(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT), VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_actualViewScreenshotImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);

    m_prevTime = glfwGetTime();

    m_numberOfViewsUsed = m_viewGrid->getViews().size();
}

void Application::draw()
{
    bool resizeViews = false;

    double lastTime = glfwGetTime();
    int frames = 0;
    int lastFps = 0;

    glm::vec2 windowResolution = m_window->getResolution();
    glm::vec2 secondaryWindowResolution = m_secondaryWindow->getResolution();
    std::shared_ptr<ViewGrid> viewGrid;
    std::shared_ptr<Framebuffer> framebuffer;

    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
        // Choose respective resources, which will be rendered mainly.
        viewGrid = m_renderFromViews ? m_viewGrid : m_novelViewGrid;
        framebuffer = m_renderFromViews ? m_renderer->getViewMatrixFramebuffer()
            : m_renderer->getOffscreenFramebuffer();

        // Consume input and set flag to change scene resources.
        if (consumeInput())
        {
            m_renderer->setSceneChanged(0);
            m_scene->setSceneChanged(true);

            if (m_novelSecondWindow)
                m_novelViewGrid->reconstructMatrices();
        }
        
        // Reconstruct matrices for the main views;
        viewGrid->reconstructMatrices();
        
        // Begin compute pass.
        m_renderer->beginComputePass();

        // Perform frustum culling when the novel view isn't rendered.
        if (!m_renderNovel)
        {
            m_renderer->cullComputePass(m_scene, viewGrid, (!m_renderFromViews));
        }
        
        // Perform compute pass for extrapolating the novel view.
        if (m_renderNovel || m_novelSecondWindow)
        {
            m_renderer->rayEvalComputePass(m_novelViewGrid, m_viewGrid, 
                RayEvalParams{m_testPixels, m_testedPixel, m_numberOfRaySamples, 
                m_automaticSampleCount, m_thresholdDepth, m_maxSampleDistance, 
                m_numberOfViewsUsed});
        }
        
        // End compute pass and submit it.
        m_renderer->endComputePass();
        m_renderer->submitCompute();

        WindowParams windowParams{m_novelSecondWindow, VK_SUCCESS, VK_SUCCESS};

        m_renderer->prepareFrame(m_scene, m_window, windowParams);
        
        if (!handlePrepareResult(windowParams, windowResolution, secondaryWindowResolution))
            continue;

        // Begin render command buffer.
        m_renderer->beginCommandBuffer();
        
        // Render triangular scene into a offscreen framebuffer.
        if (!m_renderNovel)
        {
            m_renderer->beginRenderPass(m_renderer->getOffscreenRenderPass(), framebuffer);
            m_renderer->renderPass(m_scene, viewGrid, m_viewGrid);
            m_renderer->endRenderPass();
        }

        // Image memory barrier for novel view image.
        if (m_renderNovel || m_novelSecondWindow)
            m_renderer->setNovelViewBarrier();
        
        // https://github.com/ARM-software/vulkan_best_practice_for_mobile_developers/blob/master/samples/performance/pipeline_barriers/pipeline_barriers_tutorial.md
        if (!m_renderNovel)
            m_renderer->setOffscreenFramebufferBarrier();
        
        // Renders the offscreen framebuffer or novel view into the swapchain framebuffer.
        m_renderer->beginRenderPass(m_renderer->getQuadRenderPass(), m_renderer->getQuadFramebuffer());
        m_renderer->quadRenderPass(windowResolution, m_depthOnly);

        // Renders ImGui.
        renderImgui(lastFps);
        m_renderer->endRenderPass();

        // Debug for chosen pixel.
        if (m_testPixels)
        {
            m_renderer->copyOffscreenFrameBufferToSupp();
        }

        // Renders the novel image into a secondary window.
        if (m_novelSecondWindow)
        {
            m_renderer->beginRenderPass(m_renderer->getQuadRenderPass(), m_renderer->getSecondaryQuadFramebuffer());
            m_renderer->quadRenderPass(secondaryWindowResolution, false, m_novelSecondWindow);
            m_renderer->endRenderPass();
        }

        // Ends render command buffer.
        m_renderer->endCommandBuffer();
        // Submit the frame and present it.
        m_renderer->submitFrame(m_novelSecondWindow);

        m_renderer->presentFrame(m_window, windowParams);

        if (!handlePresentResult(windowParams, windowResolution, secondaryWindowResolution))
            continue;

        // Fps counter.
        countFps(frames, lastFps, lastTime);

        handleGuiInputChanges();
    }

    vkDeviceWaitIdle(m_device->getVkDevice());
}

void Application::renderViewMatrix()
{
    m_viewGrid->reconstructMatrices();

    // Run the culling compute pass.
    m_renderer->beginComputePass();
    m_renderer->cullComputePass(m_scene, m_viewGrid, false);
    m_renderer->endComputePass();
    m_renderer->submitCompute();

    // Render the scene.
    m_renderer->beginCommandBuffer();
    m_renderer->beginRenderPass(m_renderer->getOffscreenRenderPass(), m_renderer->getViewMatrixFramebuffer());
    m_renderer->renderPass(m_scene, m_viewGrid, m_viewGrid);
    m_renderer->endRenderPass();

    // Copy the data into the test framebuffer.
    m_renderer->copyOffscreenFrameBufferToSupp();

    m_renderer->endCommandBuffer();
    m_renderer->submitGraphics();

    m_renderer->setSceneChanged(0);
    m_renderer->setLightChanged(0);

}

bool Application::consumeInput()
{
    glfwPollEvents();

    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse && !io.WantCaptureKeyboard)
    {
        float now = glfwGetTime();
        float delta = now - m_prevTime;
        m_prevTime = now;

        // for each timer do this
        m_intervalCounter -= delta;
        if (m_intervalCounter > 0.f)
        {
            return false;
        }

        m_intervalCounter = m_interval;

        VkExtent2D fbSize = m_renderer->getOffscreenFramebuffer()->getResolution();
        glm::vec2 windowSize = m_window->getResolution();
        glm::vec2 secondaryWindowSize = m_secondaryWindow->getResolution();

        bool changed = false;

        // Consume the output from the main window.
        changed = changed || vke::utils::consumeDeviceInput(m_window->getWindow(), glm::vec2(windowSize.x / fbSize.width, windowSize.y / fbSize.height),
            (m_renderFromViews) ? m_viewGrid : m_novelViewGrid, m_manipulateGrid);
        
        // Consume the input from the secondary window based - either regular input or the pixel id for evaluation.
        if (m_novelSecondWindow && !m_testPixels)
            changed = changed || vke::utils::consumeDeviceInput(m_secondaryWindow->getWindow(), glm::vec2(secondaryWindowSize.x / fbSize.width, secondaryWindowSize.y / fbSize.height),
                m_novelViewGrid, false, true);
        else if (m_novelSecondWindow && m_testPixels)
            changed = changed || vke::utils::consumeDeviceInputTestPixel(m_secondaryWindow->getWindow(), m_testedPixel);

        return changed;
    }

    return false;
}

void Application::initImgui()
{
    auto indices = vke::utils::findQueueFamilies(m_device->getPhysicalDevice(), m_device->getSurface());

    VkDescriptorPoolSize pool_sizes[] =
	{
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2}
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

    vkCreateDescriptorPool(m_device->getVkDevice(), &pool_info, nullptr, &m_imguiPool);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.DisplaySize = ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT);
    io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

    ImGui_ImplGlfw_InitForVulkan(m_window->getWindow(), true);

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_device->getInstance();
    initInfo.PhysicalDevice = m_device->getPhysicalDevice();
    initInfo.Device = m_device->getVkDevice();
    initInfo.QueueFamily = indices.graphicsFamily.value();
    initInfo.Queue = m_device->getPresentQueue();
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = m_imguiPool;
    initInfo.Allocator = NULL;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = m_renderer->getSwapChain()->getImageCount();
    initInfo.CheckVkResultFn = NULL;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo, m_renderer->getQuadRenderPass()->getRenderPass());

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
    ImGui::Text(fpsStr.c_str(), "warning fix");

    if (ImGui::Button("Screenshot"))
    {
        m_screenshot = true;
    }

    if (ImGui::CollapsingHeader("General"))
    {
        ImGui::Indent();
        ImGui::PushID(26);

        if (ImGui::Checkbox("Test pixels", &m_testPixels))
        {
            if (m_novelSecondWindow && m_renderFromViews)
            {
                m_changeOffscreenTarget = 0;
            }
            else
                m_testPixels = false;
        }

        if (!m_renderNovel)
        {
            if (ImGui::Checkbox("Depth only", &m_depthOnly))
            {
                m_changeOffscreenTarget = 0;
            }
        }

        ImGui::PopID();
        ImGui::Unindent();
    }

    if(ImGui::CollapsingHeader("Main View"))
    {
        ImGui::Indent();
        ImGui::PushID(0);        

        if (ImGui::Checkbox("Render novel view", &m_renderNovel))
        {
            if (m_renderFromViews || m_novelSecondWindow)
                m_renderNovel = false;
            else
                m_changeOffscreenTarget = 0;
        }

        bool val = m_novelSecondWindow;
        if (ImGui::Checkbox("Render novel in second window", &val))
        {
            if (!m_renderNovel)
                m_secondWindowChanged = true;

        }

        if (ImGui::CollapsingHeader("Novel view parameters"))
        {
            ImGui::Indent();

            ImGui::Checkbox("Automatic sample count", &m_automaticSampleCount);

            ImGui::Text("Number of ray samples:");
            ImGui::SliderInt("Samples", &m_numberOfRaySamples, 0, 256);

            ImGui::Text("Maximum number of views used:");
            ImGui::SliderInt("Views", &m_numberOfViewsUsed, 4, m_viewGrid->getViews().size());

            ImGui::Text("Sampling method:");
            if (ImGui::BeginCombo("##samplingCombo", samplingTypeStrings[static_cast<int>(m_samplingType)].c_str()))
            {
                for (int i = 0; i < static_cast<int>(SamplingType::END); i++)
                {
                    bool isSelected = static_cast<int>(m_samplingType) == i;
                    if (ImGui::Selectable(samplingTypeStrings[i].c_str(), isSelected))
                    {
                        m_samplingType = static_cast<SamplingType>(i);
                    }

                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }

                    if (m_renderer->getNovelViewSamplingType() != m_samplingType)
                    {
                        m_renderer->setNovelViewSamplingType(m_samplingType);
                    }
                }

                ImGui::EndCombo();
            }

            ImGui::Unindent();
        }
        
        if(ImGui::CollapsingHeader("Parameters"))
        {
            ImGui::Indent();
            if (ImGui::DragFloat("FOV", &m_mainViewFov, 1.f, 30.f, 120.f))
            {
                m_novelViewGrid->setFov(m_mainViewFov);
            }
            ImGui::Unindent();
        }

        ImGui::PopID();
        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("View Grid"))
    {
        ImGui::Indent();

        if (ImGui::Checkbox("Render from view grid", &m_renderFromViews))
        {
            if (m_renderNovel)
                m_renderFromViews = false;
            else
                m_changeOffscreenTarget = 0;

            if (!m_renderFromViews)
                m_manipulateGrid = false;
        }

        if (ImGui::Checkbox("Manipulate whole grid", &m_manipulateGrid))
        {
            if (!m_renderFromViews)
                m_manipulateGrid = false;
        }

        if (ImGui::Checkbox("Show camera geometry", &m_showCameraGeometry))
        {
            if (m_showCameraGeometry)
            {
                m_scene->addDebugCameraGeometry(m_viewGrid->getViews());
                m_scene->setSceneChanged(true);
                m_renderer->setSceneChanged(0);

            }
            else
            {
                m_scene->setRenderDebugGeometryFlag(false);
                m_scene->setSceneChanged(true);
                m_renderer->setSceneChanged(0);
            }
        }
        
        if (ImGui::CollapsingHeader("Views parameters"))
        {
            ImGui::Indent();
            ImGui::PushID(1);

            if (ImGui::DragFloat("FOV", &m_viewsFov, 1.f, 30.f, 120.f))
            {
                m_viewGrid->setFov(m_viewsFov);
                
                if (!m_renderFromViews && (m_renderNovel || m_novelSecondWindow))
                    m_reRenderViewMatrix = true;
            }

            ImGui::PopID();
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Views"))
        {
            ImGui::Indent();

            if (ImGui::Button("Add row"))
            {
                m_viewGrid->addRow();
            }

            ImGui::SameLine();

            if (ImGui::Button("Remove row"))
            {
                m_removeRow = 0;
            }

            if (ImGui::Button("Add column"))
            {
                m_viewGrid->addColumn();
            }

            ImGui::SameLine();

            if (ImGui::Button("Remove column"))
            {
                m_removeCol = 0;
            }

            int viewId = 0;
            std::vector<uint32_t> viewRowsColumns = m_viewGrid->getViewRowsColumns();
            for (int i = 0; i < viewRowsColumns.size(); i++)
            {
                std::string rowText = "Row #" + std::to_string(i);

                if (ImGui::CollapsingHeader(rowText.c_str()))
                {
                    ImGui::PushID(i);
                    ImGui::Indent();

                    std::vector<std::shared_ptr<View>> views = m_viewGrid->getViews();
                    for (int j = 0; j < viewRowsColumns[i]; j++)
                    {
                        std::string colText = "#" + std::to_string(j);
                        if (ImGui::CollapsingHeader(colText.c_str()))
                        {
                            auto& view = views[viewId + j];

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

                            ImGui::Text(rm.c_str(), "warning fix");

                            ImGui::Unindent();
                            ImGui::PopID();
                        }
                    }

                    ImGui::Unindent();
                    ImGui::PopID();
                }

                viewId += viewRowsColumns[i];
            }

            ImGui::Unindent();
        }

        ImGui::Unindent();
    }


    if (ImGui::CollapsingHeader("Light"))
    {
        glm::vec3 lightPos = m_scene->getLightPos();
        ImGui::Indent();
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
        ImGui::Unindent();
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_renderer->getCommandBuffer(m_renderer->getCurrentFrame()));
}

void Application::cleanup()
{
    
}

void Application::createMainView()
{
    utils::Config mainViewConfig = {};
    mainViewConfig.byStep = false;
    mainViewConfig.views = { m_config.novelView };
    mainViewConfig.gridFov = m_config.novelFov;

    m_mainViewFov = m_config.novelFov;

    VkExtent2D offscreenRes = m_renderer->getOffscreenFramebuffer()->getResolution();

    m_novelViewGrid = std::make_shared<ViewGrid>(m_device, glm::vec2(offscreenRes.width, offscreenRes.height), mainViewConfig,
        m_renderer->getViewDescriptorSetLayout(), m_renderer->getViewDescriptorPool(), m_cameraCube);
    m_novelViewGrid->getViews()[0]->getCamera()->setViewDir(m_config.novelView.viewDir);
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

    m_models[m_models.size() - 1]->setModelMatrix(glm::scale(glm::mat4(1.f), glm::vec3(0.1f, 0.1f, 0.1f)));

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

void Application::countFps(int& frames, int& lastFps, double& lastTime)
{
    double currentTime = glfwGetTime();
    frames++;
    if (currentTime - lastTime >= 1.f)
    {
        lastFps = frames;
        frames = 0;

        lastTime = glfwGetTime();
    }
}

void Application::handleGuiInputChanges()
{
    // Switches the source image for the on screen render pass.
    if (m_changeOffscreenTarget < MAX_FRAMES_IN_FLIGHT)
    {
        vkDeviceWaitIdle(m_device->getVkDevice());
        
        if (m_renderFromViews && !m_testPixels)
        {
            if (!m_depthOnly)
                m_renderer->changeQuadRenderPassSource(m_renderer->getViewMatrixFramebuffer()->getColorImageInfo());
            else
                m_renderer->changeQuadRenderPassSource(m_renderer->getViewMatrixFramebuffer()->getDepthImageInfo());
        }
        else if (m_renderFromViews && m_testPixels)
        {
            m_renderer->changeQuadRenderPassSource(m_renderer->getTestPixelImageInfo());
        }
        else if (m_renderNovel)
        {
            m_renderer->changeQuadRenderPassSource(m_renderer->getNovelImageInfo());
        }
        else
        {
            if (!m_depthOnly)
                m_renderer->changeQuadRenderPassSource(m_renderer->getOffscreenFramebuffer()->getColorImageInfo());
            else
                m_renderer->changeQuadRenderPassSource(m_renderer->getOffscreenFramebuffer()->getDepthImageInfo());
        }

        m_changeOffscreenTarget++;
    }

    if (m_novelSecondWindow && !m_secondaryWindow->getVisible())
    {
        m_secondWindowChanged = true;
    }

    // Turns on and off the secondary window rendering.
    if (m_secondWindowChanged)
    {
        vkDeviceWaitIdle(m_device->getVkDevice());
        m_secondWindowChanged = false;
        m_novelSecondWindow = !m_novelSecondWindow;
        m_secondaryWindow->setVisible(m_novelSecondWindow);
    }

    if (m_reRenderViewMatrix)
    {
        vkDeviceWaitIdle(m_device->getVkDevice());
        renderViewMatrix();
        m_reRenderViewMatrix = false;
        vkDeviceWaitIdle(m_device->getVkDevice());
    }

    if (m_imagesSaved && m_threadStarted)
    {
        m_saveImageThread.join();
        m_threadStarted = false;
        m_viewMatrixScreenshotImage->unmap();
        m_actualViewScreenshotImage->unmap();
        m_novelViewScreenshotImage->unmap();
    }

    if (m_screenshot && !m_threadStarted)
    {
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S/");
        std::string timeString = oss.str();
        std::string screenshotFolder = std::string(SCREENSHOT_FILES_LOC) + timeString;
        std::filesystem::create_directories(screenshotFolder);

        VkCommandBuffer commandBuffer;
        m_device->beginSingleCommands(commandBuffer);

        m_device->copyImageToImage(m_renderer->getViewMatrixFramebuffer()->getColorImage(), m_viewMatrixScreenshotImage,
            commandBuffer);
        
        m_device->copyImageToImage(m_renderer->getOffscreenFramebuffer()->getColorImage(), m_actualViewScreenshotImage,
            commandBuffer);

        m_device->copyImageToImage(m_renderer->getNovelViewImage(), m_novelViewScreenshotImage, commandBuffer);

        m_device->endSingleCommands(commandBuffer);

        m_viewMatrixScreenshotImage->map();
        void* viewMatrixData = m_viewMatrixScreenshotImage->getMapped();
        glm::ivec3 viewMatrixDims = glm::ivec3(m_viewMatrixScreenshotImage->getDims(), 4);

        m_actualViewScreenshotImage->map();
        void* actualViewData = m_actualViewScreenshotImage->getMapped();
        glm::ivec3 actualViewDims = glm::ivec3(m_actualViewScreenshotImage->getDims(), 4);

        m_novelViewScreenshotImage->map();
        void* novelData = m_novelViewScreenshotImage->getMapped();
        glm::ivec3 novelDims = glm::ivec3(m_novelViewScreenshotImage->getDims(), 4);

        std::vector<SaveImageInfo> saveImageInfos = {
            SaveImageInfo{screenshotFolder + "viewMatrix.ppm", viewMatrixDims, (uint8_t*)viewMatrixData},
            SaveImageInfo{screenshotFolder + "gtView.ppm", actualViewDims, (uint8_t*)actualViewData},
            SaveImageInfo{screenshotFolder + "novelView.ppm", novelDims, (uint8_t*)novelData}
        };

        m_imagesSaved = false;
        m_threadStarted = true;
        m_saveImageThread = std::thread(vke::utils::saveImagesThread, saveImageInfos, std::ref(m_imagesSaved));

        vke::utils::saveConfig(screenshotFolder + "config.json", m_config, m_novelViewGrid, m_viewGrid);

        m_screenshot = false;
    }

    if (m_removeRow < MAX_FRAMES_IN_FLIGHT)
    {
        vkDeviceWaitIdle(m_device->getVkDevice());
        m_viewGrid->removeRow(!m_renderer->getCurrentFrame(), (m_removeRow == 0));
        m_removeRow++;
    }

    if (m_removeCol < MAX_FRAMES_IN_FLIGHT)
    {
        vkDeviceWaitIdle(m_device->getVkDevice());
        m_viewGrid->removeColumn(!m_renderer->getCurrentFrame(), (m_removeCol == 0));
        m_removeCol++;
    }
}

bool Application::handlePrepareResult(WindowParams& params, glm::vec2& windowResolution,
    glm::vec2& secondaryWindowResolution)
{
    if (params.result == VK_ERROR_OUT_OF_DATE_KHR || 
        params.secondaryResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        if (params.result == VK_ERROR_OUT_OF_DATE_KHR)
            m_renderer->handleResizeWindow();
        
        if (params.secondaryResult == VK_ERROR_OUT_OF_DATE_KHR)
            m_renderer->handleResizeWindow(false);
        
        windowResolution = m_window->getResolution();
        secondaryWindowResolution = m_secondaryWindow->getResolution();

        // recreate swap
        params.result = VK_SUCCESS;
        params.secondaryResult = VK_SUCCESS;

        return false;
    }
    else if ((params.result != VK_SUCCESS && params.result != VK_SUBOPTIMAL_KHR) || 
                (params.secondaryResult != VK_SUCCESS && params.secondaryResult != VK_SUBOPTIMAL_KHR))
    {
        throw std::runtime_error("Error: failed to acquire swap chain image.");
    }

    return true;
}

bool Application::handlePresentResult(WindowParams &params, glm::vec2& windowResolution,
    glm::vec2& secondaryWindowResolution)
{
    if (params.result == VK_ERROR_OUT_OF_DATE_KHR || params.result == VK_SUBOPTIMAL_KHR ||
        m_window->resized() || m_secondaryWindow->resized())
    {            
        m_renderer->handleResizeWindow();

        if (m_novelSecondWindow)
            m_renderer->handleResizeWindow(false);
        
        windowResolution = m_window->getResolution();
        secondaryWindowResolution = m_secondaryWindow->getResolution();

        // recreate swap
        params.result = VK_SUCCESS;
        params.secondaryResult = VK_SUCCESS;
        return false;
    }
    else if (params.result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image");
    }

    return true;
}

}