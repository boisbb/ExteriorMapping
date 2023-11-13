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


bool enableValidationLayers = true;

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
    std::cout << "renderer" << std::endl;
    m_renderer = std::make_shared<Renderer>(m_device, m_window, "../res/shaders/vert.spv",
        "../res/shaders/frag.spv", "../res/shaders/comp.spv");
    m_scene = std::make_shared<Scene>();

    glm::vec3 lightPos = { 0.f, 10.f, 0.f };
    m_scene->setLightPos(lightPos);

    glm::vec2 swapExtent((float)m_renderer->getSwapChain()->getExtent().width,
        (float)m_renderer->getSwapChain()->getExtent().height);

    m_camera = std::make_shared<Camera>(swapExtent, glm::vec3(2.f, 10.f, 2.f));

    std::shared_ptr<Model> negus = vke::utils::importModel("../res/models/negusPlane/negusPlane.obj",
        m_vertices, m_indices);
    negus->afterImportInit(m_device, m_renderer);

    std::shared_ptr<Model> pepe = vke::utils::importModel("../res/models/pepePlane/pepePlane.obj",
        m_vertices, m_indices);
    pepe->afterImportInit(m_device, m_renderer);

    std::shared_ptr<Model> porsche = vke::utils::importModel("../res/models/porsche/porsche.obj",
        m_vertices, m_indices);
    porsche->afterImportInit(m_device, m_renderer);

    std::shared_ptr<Model> sponza = vke::utils::importModel("../res/models/dabrovic_sponza/sponza.obj",
        m_vertices, m_indices);
    sponza->afterImportInit(m_device, m_renderer);

    m_light = vke::utils::importModel("../res/models/basicCube/cube.obj",
        m_vertices, m_indices);
    m_light->afterImportInit(m_device, m_renderer);
    glm::mat4 lightMatrix = m_light->getModelMatrix();
    lightMatrix = glm::translate(lightMatrix, lightPos);
    lightMatrix = glm::scale(lightMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
    m_light->setModelMatrix(lightMatrix);

    m_models.push_back(sponza);
    m_models.push_back(porsche);
    //m_models.push_back(pepe);
    // m_models.push_back(negus);
    //m_models.push_back(m_light);

    m_scene->setModels(m_device, m_renderer->getComputeDescriptorSetLayout(),
        m_renderer->getComputeDescriptorPool(), m_models, m_vertices, m_indices);

    m_renderer->initDescriptorResources();

    initImgui();
}

void Application::draw()
{

#if PRINT_FPS
    int frames = 0;
    auto start = high_resolution_clock::now();
#endif

    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
        consumeInput();

        m_renderer->renderFrame(m_scene, m_camera);

        // uint32_t imageIndex = m_renderer->prepareFrame(m_scene, m_camera);
// 
        // m_renderer->recordCommandBuffer(m_renderer->getCurrentCommandBuffer(), m_scene, imageIndex);
        // renderImgui();
// 
        // m_renderer->presentFrame(imageIndex);

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
        vke::utils::consumeDeviceInput(m_window->getWindow(), m_camera);
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

    glm::vec3 lightPos = m_scene->getLightPos();
    if (ImGui::SliderFloat3("Light Position:", &lightPos.x, -50.f, 50.f))
    {
        m_scene->setLightPos(lightPos);
        glm::mat4 lightMatrix = glm::translate(glm::mat4(1.f), lightPos);
        lightMatrix = glm::scale(lightMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
        m_light->setModelMatrix(lightMatrix);
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_renderer->getCommandBuffer(m_renderer->getCurrentFrame()));
}

void Application::cleanup()
{
    
}

}