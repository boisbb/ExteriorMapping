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

#define PRINT_FPS false
#define DRAW_LIGHT false


bool enableValidationLayers = true;

using namespace std::chrono;

namespace vke
{

Application::Application()
    : m_viewRowColumns(1, 0)
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
        "../res/shaders/frag.spv", "../res/shaders/comp.spv");
    m_scene = std::make_shared<Scene>();

    glm::vec3 lightPos = { 0.f, 10.f, 0.f };
    m_scene->setLightPos(lightPos);

    glm::vec2 swapExtent((float)m_renderer->getSwapChain()->getExtent().width,
        (float)m_renderer->getSwapChain()->getExtent().height);

    //std::shared_ptr<Model> negus = vke::utils::importModel("../res/models/negusPlane/negusPlane.obj",
    //    m_vertices, m_indices);
    //negus->afterImportInit(m_device, m_renderer);
    //
    //std::shared_ptr<Model> pepe = vke::utils::importModel("../res/models/pepePlane/pepePlane.obj",
    //    m_vertices, m_indices);
    //pepe->afterImportInit(m_device, m_renderer);
    //
    //std::shared_ptr<Model> sphereG = vke::utils::importModel("../res/models/basicSphere/basicSphere.obj",
    //    m_vertices, m_indices);
    //sphereG->afterImportInit(m_device, m_renderer);
    //

    std::shared_ptr<Model> porsche = vke::utils::importModel("../res/models/porsche/porsche.obj",
        m_vertices, m_indices);
    porsche->afterImportInit(m_device, m_renderer);
    
    std::shared_ptr<Model> sponza = vke::utils::importModel("../res/models/dabrovic_sponza/sponza.obj",
        m_vertices, m_indices);
    sponza->afterImportInit(m_device, m_renderer);

#if DRAW_LIGHT
    m_light = vke::utils::importModel("../res/models/basicCube/cube.obj",
        m_vertices, m_indices);
    m_light->afterImportInit(m_device, m_renderer);
    glm::mat4 lightMatrix = m_light->getModelMatrix();
    lightMatrix = glm::translate(lightMatrix, lightPos);
    lightMatrix = glm::scale(lightMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
    m_light->setModelMatrix(lightMatrix);
#endif


    m_models.push_back(sponza);
    m_models.push_back(porsche);
    //m_models.push_back(pepe);
    //m_models.push_back(negus);
    //m_models.push_back(m_light);
    //m_models.push_back(sphereG);

    // size_t modelSize = m_models.size();
    // for (size_t i = 0; i < modelSize; i++)
    // {
    //     for (auto& mesh : m_models[i]->getMeshes())
    //     {
    //         glm::vec3 center = mesh->getBbCenter();
    //         float radius = mesh->getBbRadius();
    // 
    //         std::shared_ptr<Model> sphere = vke::utils::importModel("../res/models/basicSphere/basicSphere.obj",
    //             m_vertices, m_indices);
    //         sphere->afterImportInit(m_device, m_renderer);
    //         glm::mat4 newMat = glm::translate(glm::mat4{1.f}, center);
    //         newMat = glm::scale(newMat, glm::vec3(radius));
    //         sphere->setModelMatrix(newMat);
    // 
    //         m_models.push_back(sphere);
    //     }
    // }

    m_scene->setModels(m_device, m_renderer->getSceneComputeDescriptorSetLayout(),
        m_renderer->getSceneComputeDescriptorPool(), m_models, m_vertices, m_indices);

    // std::shared_ptr<View> view = std::make_shared<View>(glm::vec2(WIDTH, HEIGHT), glm::vec2(0.f, 0.f),
    //     m_device, m_renderer->getViewDescriptorSetLayout(), m_renderer->getViewDescriptorPool());

    addViewColumn(0, 0);

    m_renderer->initDescriptorResources();

    initImgui();
}

void Application::draw()
{

#if PRINT_FPS
    int frames = 0;
    auto start = high_resolution_clock::now();
#endif
    bool resizeViews = false;

    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
        consumeInput();

        m_renderer->computePass(m_scene, m_views);

        uint32_t imageIndex = m_renderer->renderPass(m_scene, m_views);

        m_renderer->beginRenderPass(m_views[0], m_renderer->getCurrentFrame(), imageIndex, false);
        renderImgui();
        m_renderer->endRenderPass(m_renderer->getCurrentFrame());

        m_renderer->endCommandBuffer();
        m_renderer->submitFrame();
        m_renderer->presentFrame(imageIndex, m_window, nullptr, resizeViews);



#if PRINT_FPS
        frames++;
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        int elapsedMs = duration.count();
        float elapsedS = static_cast<float>(elapsedMs) / 1000.f;
        float fps = static_cast<float>(frames) / elapsedS;
        std::cout << fps << std::endl;
#endif
        
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

void Application::renderImgui()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Config");

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
            std::cout << "Delete View row" << std::endl;
        }
        ImGui::SameLine();
        if (ImGui::Button("+"))
        {
            std::cout << "Add View row" << std::endl;
        }

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
                    std::cout << "Delete row view " << i <<std::endl;
                }

                ImGui::SameLine();

                if (ImGui::Button("+"))
                {
                    std::cout << "Add row view " << i <<std::endl;
                    addViewColumn(i, viewId);
                }

                for (int j = 0; j < m_viewRowColumns[i]; j++)
                {
                    std::string colText = "#" + std::to_string(j);
                    if (ImGui::CollapsingHeader(colText.c_str()))
                    {
                        auto& view = m_views[viewId];

                        ImGui::PushID(j);
                        ImGui::Indent();

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
    VkExtent2D windowExtent = m_window->getExtent();

    int rowViewsCount = m_viewRowColumns[rowId];

    int newViewWidth = static_cast<float>(windowExtent.width) / static_cast<float>(rowViewsCount + 1);
    int newViewHeight = static_cast<float>(windowExtent.height) / static_cast<float>(m_viewRowColumns.size());

    int newViewWidthOffset = 0;
    int newViewHeightOffset = rowId * newViewHeight;

    std::cout << std::endl;
    std::cout << rowViewStartId << std::endl;
    std::cout << newViewWidthOffset << std::endl;
    std::cout << newViewWidth << std::endl;
    std::cout << newViewHeight << std::endl;

    for (int i = 0; i < rowViewsCount; i++)
    {
        newViewWidthOffset = newViewWidth * i;
        std::cout << newViewWidthOffset << std::endl;

        m_views[rowViewStartId + i]->setResolution(glm::vec2(newViewWidth, newViewHeight));
        m_views[rowViewStartId + i]->setViewportStart(glm::vec2(newViewWidthOffset, newViewHeightOffset));
        
    }

    newViewWidthOffset = newViewWidth * rowViewsCount;

    std::shared_ptr<View> view = std::make_shared<View>(glm::vec2(newViewWidth, newViewHeight), glm::vec2(newViewWidthOffset, newViewHeightOffset),
        m_device, m_renderer->getViewDescriptorSetLayout(), m_renderer->getViewDescriptorPool());
    
    m_views.insert(m_views.begin() + rowViewsCount, view);

    m_viewRowColumns[rowId] += 1;
}

}