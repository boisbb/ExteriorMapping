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

#define DRAW_LIGHT false

using namespace std::chrono;

std::array<std::string, 3> samplingTypeStrings = {
    "color",
    "depth with distance",
    "depth with angle distance"
};

namespace vke
{

Application::Application()
    : m_showCameraGeometry(false),
    m_renderFromViews(false),
    m_changeOffscreenTarget(MAX_FRAMES_IN_FLIGHT),
    m_samplingType(SamplingType::COLOR)
{
    init();
}

void Application::run()
{
    draw();
}

void Application::init()
{
    // utils::parseConfig("../res/configs/full_grid/config6grid.json", m_config);
    utils::parseConfig("../res/configs/by_step/config.json", m_config);

    m_window = std::make_shared<Window>(WINDOW_WIDTH, WINDOW_HEIGHT);

    m_device = std::make_shared<Device>(m_window);
    m_renderer = std::make_shared<Renderer>(m_device, m_window, "offscreen.vert.spv",
        "offscreen.frag.spv", "cull.comp.spv", "quad.vert.spv",
        "quad.frag.spv", "novelView.comp.spv");
    m_renderer->setNovelViewSamplingType(m_samplingType);

    m_window2 = std::make_shared<Window>(WINDOW_WIDTH, WINDOW_HEIGHT, false);
    m_window2->createWindowSurface(m_device->getInstance());
    m_renderer->addSecondaryWindow(m_window2);

    createScene();

    createMainView();

    VkExtent2D offscreenRes = m_renderer->getOffscreenFramebuffer()->getResolution();

    m_viewGrid = std::make_shared<ViewGrid>(m_device, glm::vec2(offscreenRes.width, offscreenRes.height), m_config, 
        m_renderer->getViewDescriptorSetLayout(), m_renderer->getViewDescriptorPool(), m_cameraCube);
    
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

    preRender();

    vkDeviceWaitIdle(m_device->getVkDevice());

    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
        windowResolution = m_window->getResolution();

        std::shared_ptr<ViewGrid> viewGrid = m_renderFromViews ? m_viewGrid : m_novelViewGrid;
        std::shared_ptr<Framebuffer> framebuffer = m_renderFromViews ? m_renderer->getViewMatrixFramebuffer()
            : m_renderer->getOffscreenFramebuffer();

        if (consumeInput())
        {
            m_renderer->setSceneChanged(0);
            m_scene->setSceneChanged(true);
        }

        viewGrid->reconstructMatrices();
        
        m_renderer->beginComputePass();

        if (!m_renderNovel)
        {
            m_renderer->cullComputePass(m_scene, viewGrid, (!m_renderFromViews));
        }
        
        if (m_renderNovel || m_novelSecondWindow)
        {
            m_renderer->rayEvalComputePass(m_novelViewGrid, m_viewGrid);
        }
        
        m_renderer->endComputePass();
        m_renderer->submitCompute();
        
        m_renderer->prepareFrame(m_scene, nullptr, m_window, resizeViews, m_novelSecondWindow);

        m_renderer->beginCommandBuffer();
        
        if (!m_renderNovel)
        {
            m_renderer->beginRenderPass(m_renderer->getOffscreenRenderPass(), framebuffer);
            m_renderer->renderPass(m_scene, viewGrid, m_viewGrid);
            m_renderer->endRenderPass();

            m_renderer->beginRenderPass(m_renderer->getOffscreenRenderPass(), m_renderer->getOffscreenSuppFramebuffer());
            m_renderer->renderPass(m_scene, viewGrid, m_viewGrid, false);
            m_renderer->endRenderPass();
        }

        if (m_renderNovel || m_novelSecondWindow)
        {
            m_novelViewGrid->getViews()[0]->getCamera()->reconstructMatrices();
            m_novelViewGrid->getViews()[0]->updateDescriptorData(m_renderer->getCurrentFrame());
        }

        if (m_renderNovel || m_novelSecondWindow)
            m_renderer->setNovelViewBarrier();
        
        m_renderer->beginRenderPass(m_renderer->getQuadRenderPass(), m_renderer->getQuadFramebuffer());
        m_renderer->quadRenderPass(windowResolution, m_depthOnly);
        renderImgui(lastFps);
        m_renderer->endRenderPass();

        if (m_novelSecondWindow)
        {
            m_renderer->beginRenderPass(m_renderer->getQuadRenderPass(), m_renderer->getSecondaryQuadFramebuffer());
            m_renderer->quadRenderPass(windowResolution, false, m_novelSecondWindow);
            m_renderer->endRenderPass();
        }

        m_renderer->endCommandBuffer();

        m_renderer->submitFrame(m_novelSecondWindow);
        m_renderer->presentFrame(m_window, nullptr, resizeViews, m_novelSecondWindow);

        if (m_rayEvalOnCpu)
            mainCameraTestRays();

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

        static int test = 0;
        if (test < MAX_FRAMES_IN_FLIGHT)
        {
            vkDeviceWaitIdle(m_device->getVkDevice());

            m_renderer->changeQuadRenderPassSource(m_renderer->getOffscreenSuppFramebuffer()->getColorImageInfo());
            test++;
        }

        if (m_changeOffscreenTarget < MAX_FRAMES_IN_FLIGHT)
        {
            vkDeviceWaitIdle(m_device->getVkDevice());
            
            if (m_renderFromViews)
            {
                if (!m_depthOnly)
                    m_renderer->changeQuadRenderPassSource(m_renderer->getViewMatrixFramebuffer()->getColorImageInfo());
                else
                    m_renderer->changeQuadRenderPassSource(m_renderer->getViewMatrixFramebuffer()->getDepthImageInfo());
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

        if (m_secondWindowChanged)
        {
            vkDeviceWaitIdle(m_device->getVkDevice());
            m_secondWindowChanged = false;
            m_novelSecondWindow = !m_novelSecondWindow;
            m_window2->setVisible(m_novelSecondWindow);
        }
    }

    vkDeviceWaitIdle(m_device->getVkDevice());
}

void Application::preRender()
{
    m_viewGrid->reconstructMatrices();

    m_renderer->beginComputePass();
    m_renderer->cullComputePass(m_scene, m_viewGrid, false);
    m_renderer->endComputePass();
    m_renderer->submitCompute();

    m_renderer->beginCommandBuffer();
    m_renderer->beginRenderPass(m_renderer->getOffscreenRenderPass(), m_renderer->getViewMatrixFramebuffer());
    m_renderer->renderPass(m_scene, m_viewGrid, m_viewGrid);
    m_renderer->endRenderPass();
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
        VkExtent2D fbSize = m_renderer->getOffscreenFramebuffer()->getResolution();
        glm::vec2 windowSize = m_window->getResolution();

        bool changed = false;

        changed = changed || vke::utils::consumeDeviceInput(m_window->getWindow(), glm::vec2(windowSize.x / fbSize.width, windowSize.y / fbSize.height),
            (m_renderFromViews) ? m_viewGrid : m_novelViewGrid, m_manipulateGrid);
        
        if (m_novelSecondWindow)
            changed = changed || vke::utils::consumeDeviceInput(m_window2->getWindow(), glm::vec2(windowSize.x / fbSize.width, windowSize.y / fbSize.height),
                m_novelViewGrid, false, true);

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

    VkDescriptorPool imguiPool;
    vkCreateDescriptorPool(m_device->getVkDevice(), &pool_info, nullptr, &imguiPool);

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
    initInfo.DescriptorPool = imguiPool;
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
    ImGui::Text(fpsStr.c_str());

    if (ImGui::CollapsingHeader("General"))
    {
        ImGui::Indent();
        ImGui::PushID(26);

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

    if(ImGui::CollapsingHeader("Main view"))
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
        
        //ImGui::DragFloat("Views FOV", &m_viewsFov, 5.f, 0.f, 120.f);
        if(ImGui::CollapsingHeader("Parameters"))
        {
            ImGui::InputFloat("FOV", &m_mainViewFov);
            if(ImGui::Button("Apply"))
            {
                m_novelViewGrid->getViews()[0]->getCamera()->setFov(m_mainViewFov);
            }
        }

        ImGui::PopID();
        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Views"))
    {
        ImGui::Indent();

        if (ImGui::Checkbox("Render from view matrix", &m_renderFromViews))
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

        ImGui::Checkbox("Calculate ray eval on cpu", &m_rayEvalOnCpu);
        
        if (ImGui::CollapsingHeader("Views parameters"))
        {
            ImGui::Indent();
            ImGui::PushID(1);
            
            //ImGui::DragFloat("Views FOV", &m_viewsFov, 5.f, 0.f, 120.f);
            ImGui::InputFloat("FOV", &m_viewsFov);
            if(ImGui::Button("Apply"))
            {
                for (auto& view : m_views)
                {
                    view->getCamera()->setFov(m_viewsFov);
                }
            }

            ImGui::PopID();
            ImGui::Unindent();
        }

        if (ImGui::Button("-"))
        {
            //removeViewRow();
            m_scene->setReinitializeDebugCameraGeometryFlag(true);
            m_scene->addDebugCameraGeometry(m_views);
        }
        ImGui::SameLine();
        if (ImGui::Button("+"))
        {
            //addViewRow();
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
                //removeViewColumn(info.rowId, info.viewId + addViewId);
                addViewId -= 1;
                remove = true;
            }
            else
            {
                //addViewColumn(info.rowId, info.viewId + addViewId);
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

struct IntersectInfo
{
    glm::vec3 coords;
    float t;
    bool in;
    int planeId;
    std::vector<float> inOut;
};

struct IntervalHit {
    glm::vec2 t;
    unsigned int idBits[4];
};

glm::vec2 calculateMaskId(float id)
{
    int maskId = int(floor(id / 32));
    int innermaskId = int(id) - (maskId * 32);

    return glm::vec2(maskId, innermaskId);
}

void Application::mainCameraTestRays()
{
    std::shared_ptr<Camera> mainCamera = m_novelViewGrid->getViews()[0]->getCamera();
    glm::vec2 res = m_novelViewGrid->getViews()[0]->getResolution();
    glm::vec2 mainCameraRes = mainCamera->getResolution();

    FrustumHit frustumHitsIn[128];
    FrustumHit frustumHitsOut[128];

    unsigned int intersectCount = 0;
    
    for (int y = 0; y < mainCameraRes.y; y++)
    {
        if (y != std::round(res.y / 2.f))
            continue;
        for (int x = 0; x < mainCameraRes.x; x++)
        {
            if (x != std::round(res.x / 2.f))
                continue;

            glm::vec2 pixelCenter(x + 0.5f, y + 0.5f);
            glm::vec2 uv = pixelCenter / mainCameraRes;

            glm::vec2 d = uv * 2.f - 1.f;

            glm::vec4 from = mainCamera->getProjectionInverse() * glm::vec4(0.f, 0.f, 0.f, 1.f);
            glm::vec4 target = mainCamera->getProjectionInverse() * glm::vec4(d.x, d.y, 1.f, 1.f);

            from /= from.w;
            target /= target.w;

            glm::vec3 org = (mainCamera->getViewInverse() * glm::vec4(from)).xyz();
            glm::vec3 dir = (mainCamera->getViewInverse() * glm::vec4(glm::normalize(target.xyz()), 0.f)).xyz();

            for (int i = 0; i < m_views.size(); i++)
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

                if (intersects.size() == 2)
                {
                    frustumHitsIn[intersectCount].viewId = i;
                    frustumHitsOut[intersectCount].viewId = i;

                    if (intersects[0].t < intersects[1].t)
                    {
                        frustumHitsIn[intersectCount].t = intersects[0].t;
                        frustumHitsOut[intersectCount].t = intersects[1].t;
                    }
                    else
                    {
                        frustumHitsIn[intersectCount].t = intersects[1].t;
                        frustumHitsOut[intersectCount].t = intersects[0].t;
                    }

                    std::cout << "      view: " << i << std::endl;
                    std::cout << "      t0: " << frustumHitsIn[intersectCount].t << std::endl;
                    std::cout << "      t1: " << frustumHitsOut[intersectCount].t << std::endl;

                    intersectCount++;
                    // intersectsMap[i][std::pair<int, int>(x, y)] = intersects;
                }
            }

            // insert sort the intersections
            for (int i = 1; i < intersectCount; i++)
            {
                // hits in
                FrustumHit keyIn = frustumHitsIn[i];

                int j = i - 1;
                while (j >= 0 && frustumHitsIn[j].t > keyIn.t)
                {
                    frustumHitsIn[j + 1] = frustumHitsIn[j];
                    j = j - 1;
                }

                frustumHitsIn[j + 1] = keyIn;

                // hits out
                FrustumHit keyOut = frustumHitsOut[i];

                j = i - 1;
                while (j >= 0 && frustumHitsOut[j].t > keyOut.t)
                {
                    frustumHitsOut[j + 1] = frustumHitsOut[j];
                    j = j - 1;
                }

                frustumHitsOut[j + 1] = keyOut;
            }

            IntervalHit intervals[32];

            int currentlyInInterval = 0;
            unsigned int cameraIndexMask[4];
            cameraIndexMask[0] = 0;
            cameraIndexMask[1] = 0;
            cameraIndexMask[2] = 0;
            cameraIndexMask[3] = 0;

            float currentStartT = 0;

            int foundIntervals = 0;
            int inId = 0;
            int outId = 0;
            for (int i = 0; i < intersectCount * 2; i++)
            {
                FrustumHit hitIn = frustumHitsIn[std::min(inId, 64)];
                FrustumHit hitOut = frustumHitsOut[outId];

                if (hitIn.t <= hitOut.t && inId < intersectCount)
                {
                    // add into interval
                    currentlyInInterval++;

                    glm::vec2 maskId = calculateMaskId(hitIn.viewId);
                    int outer = int(maskId.x);
                    int inner = int(maskId.y);

                    cameraIndexMask[outer] = cameraIndexMask[outer] | (1 << inner);

                    currentStartT = hitIn.t;

                    inId++;
                }
                else
                {
                    // remove from interval and possibly write interval
                    if (currentlyInInterval >= 4)
                    {
                        bool alreadyIn = false;
                        for (int j = 0; j < foundIntervals; j++)
                        {
                            IntervalHit current = intervals[j];

                            bool currentSame = true;
                            bool newSame = true;
                            for (int k = 0; k < 4; k++)
                            {
                                unsigned int mask = current.idBits[k] & cameraIndexMask[k];

                                if (mask != current.idBits[k])
                                    currentSame = false;
                                
                                if (mask != cameraIndexMask[k])
                                    newSame = false;

                            }

                            if (currentSame && newSame)
                            {
                                alreadyIn = true;
                                break;
                            }
                            else if (currentSame)
                            {
                                intervals[j].t = glm::vec2(currentStartT, hitOut.t);
                                
                                for (int k = 0; k < 4; k++)
                                {
                                    intervals[j].idBits[k] = cameraIndexMask[k];
                                }
                            }
                            else if (newSame)
                            {
                                alreadyIn = true;
                                break;
                            }
                        }

                        if (!alreadyIn)
                        {
                            intervals[foundIntervals].t = glm::vec2(currentStartT, hitOut.t);
                            intervals[foundIntervals].idBits[0] = cameraIndexMask[0];
                            intervals[foundIntervals].idBits[1] = cameraIndexMask[1];
                            intervals[foundIntervals].idBits[2] = cameraIndexMask[2];
                            intervals[foundIntervals].idBits[3] = cameraIndexMask[3];
                            foundIntervals++;
                        }
                    }

                    currentlyInInterval--;

                    glm::vec2 maskId = calculateMaskId(hitOut.viewId);
                    int outer = int(maskId.x);
                    int inner = int(maskId.y);

                    cameraIndexMask[outer] = cameraIndexMask[outer] & (~(1 << inner));

                    // cycle until we find a previous value in the interval
                    for (int j = inId; j >= 0; j--)
                    {
                        FrustumHit prevHitIn = frustumHitsIn[j];
                        
                        if (prevHitIn.t == currentStartT && prevHitIn.viewId != hitOut.viewId)
                        {
                            break;
                        }
                        else if (prevHitIn.t == currentStartT && prevHitIn.viewId == hitOut.viewId)
                        {
                            continue;
                        }
                        else
                        {
                            glm::vec2 prevMaskId = calculateMaskId(prevHitIn.viewId);
                            int prevOuter = int(maskId.x);
                            int prevInner = int(maskId.y);

                            unsigned int prevIdNum = (1 << prevInner);
                            unsigned int check = cameraIndexMask[prevOuter] & prevIdNum;

                            if (check == prevIdNum)
                            {
                                // it is in the interval
                                currentStartT = prevHitIn.t;
                                break;
                            }
                        }
                    }

                    outId++;
                }
            }

            for (int i = 0; i < foundIntervals; i++)
            {
                std::cout << "Debug cpu info:" << std::endl;
                std::cout << "number of intersections: " << intersectCount << std::endl;
                std::cout << "number of intervals: " << foundIntervals << std::endl;
                
                std::cout << "interval: " << intervals[i].t.x << " " << intervals[i].t.y << std::endl;
                
                for (int j = 0; j < 128; j++)
                {
                    glm::vec2 maskId = calculateMaskId(j);
                    int outer = int(maskId.x);
                    int inner = int(maskId.y);

                    unsigned check = intervals[i].idBits[outer] & (1 << inner);

                    if (check == (1 << inner))
                    {
                        glm::mat4 view = m_views[j]->getCamera()->getView();
                        glm::mat4 proj = m_views[j]->getCamera()->getProjection();
                        glm::vec2 res = m_views[j]->getResolution();

                        glm::vec3 t1 = org + dir * intervals[i].t.x;
                        glm::vec3 t2 = org + dir * intervals[i].t.y;

                        glm::vec4 ct1 = proj * view * glm::vec4(t1.x, t1.y, t1.z, 1.f);
                        glm::vec4 ct2 = proj * view * glm::vec4(t2.x, t2.y, t2.z, 1.f);

                        ct1 /= ct1.w;
                        ct2 /= ct2.w;

                        glm::vec2 uv1(ct1.x, ct1.y);
                        glm::vec2 uv2(ct2.x, ct2.y);

                        uv1 = ((uv1 + 1.f) / 2.f) * res;
                        uv2 = ((uv2 + 1.f) / 2.f) * res;

                        // std::cout << "cam: " << j << " pixels: " << 
                    }
                }
            }
        }


    }
}

}