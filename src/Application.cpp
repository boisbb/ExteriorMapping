#include "Application.h"
#include "utils/Import.h"
#include "utils/Callbacks.h"
#include "utils/Constants.h"
#include "utils/Structs.h"
#include "utils/Input.h"

// std
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <set>
#include <limits>
#include <algorithm>

#include <chrono>

#include <glm/gtc/matrix_transform.hpp>

bool enableValidationLayers = true;

uint32_t currentFrame = 0;

namespace vke
{

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f , 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f  , 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f , 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint32_t> indices = {
    0, 1, 2,
    2, 3, 0
};

void Application::run()
{
    init();
}

void Application::init()
{
    m_window = std::make_shared<Window>(WIDTH, HEIGHT);
    m_device = std::make_shared<Device>(m_window);
    m_renderer = std::make_shared<Renderer>(m_device, m_window);
    // m_swapChain = std::make_shared<SwapChain>(m_device, m_window->getExtent());

    ubos.resize(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        ubos[i] = std::make_unique<Buffer>(m_device, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        ubos[i]->map();
    }

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> layoutB;
    layoutB.push_back(uboLayoutBinding);
    std::shared_ptr<DescriptorSetLayout> m_dSetLayout = std::make_shared<DescriptorSetLayout>(m_device, layoutB);

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    std::vector<VkDescriptorPoolSize> poolS;
    poolS.push_back(poolSize);
    std::shared_ptr<DescriptorPool> m_dPool = std::make_shared<DescriptorPool>(m_device, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), 0, poolS);

    m_dSets.resize(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        auto bufferInfo = ubos[i]->getInfo();
        m_dSets[i] = std::make_shared<DescriptorSet>(m_device, m_dSetLayout, m_dPool, 0, &bufferInfo);
    }

    VkDescriptorSetLayout vkLayout = m_dSetLayout->getLayout();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &vkLayout;

    if (vkCreatePipelineLayout(m_device->getVkDevice(), &pipelineLayoutInfo, nullptr, &m_vkPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    m_pipeline = std::make_shared<Pipeline>(m_device, m_renderer->getSwapChain()->getRenderPass(), "../res/shaders/vert.spv",
        "../res/shaders/frag.spv", m_vkPipelineLayout);

    
    // m_model = std::make_shared<Model>();
    // std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(vertices, indices);
    // m_model->addMesh(mesh);
    // m_model->afterImportInit(m_device);
    
    m_model = vke::utils::importModel("../res/models/basicCube/cube.obj");
    m_model->afterImportInit(m_device);

    glm::vec2 swapExtent((float)m_renderer->getSwapChain()->getExtent().width,
        (float)m_renderer->getSwapChain()->getExtent().height);

    m_camera = std::make_shared<Camera>(swapExtent, glm::vec3(2.f, 2.f, 2.f));

    // createVertexBuffer();
    // createIndexBuffer();

    std::cout << "init done" << std::endl;

    while(!glfwWindowShouldClose(m_window->getWindow()))
    {
        vke::utils::consumeDeviceInput(m_window->getWindow(), m_camera);
        drawFrame();
        glfwPollEvents();
    }

    vkDeviceWaitIdle(m_device->getVkDevice());
}

void Application::drawFrame()
{
    m_camera->reconstructMatrices();
    
    VkCommandBuffer currentCBuffer = m_renderer->getCommandBuffer(currentFrame);

    VkFence currentFence = m_renderer->getSwapChain()->getFenceId(currentFrame);
    vkWaitForFences(m_device->getVkDevice(), 1, &currentFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkSemaphore currentImageAvailableSemaphore = m_renderer->getSwapChain()->getImageAvailableSemaphore(currentFrame);
    VkResult result = vkAcquireNextImageKHR(m_device->getVkDevice(), m_renderer->getSwapChain()->getSwapChain(), UINT64_MAX, currentImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // recreateSwapChain();
        throw std::runtime_error("OUT OF DATE KHR");
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    vkResetFences(m_device->getVkDevice(), 1, &currentFence);

    vkResetCommandBuffer(currentCBuffer, 0);
    recordCommandBuffer(currentCBuffer, imageIndex);

    updateUniformBuffer(currentFrame);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemamphores[] = {currentImageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemamphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentCBuffer;

    VkSemaphore currentRenderFinishedSemaphore = m_renderer->getSwapChain()->getRenderFinishedSemaphore(currentFrame);
    VkSemaphore signalSemaphores[] = {currentRenderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, currentFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_renderer->getSwapChain()->getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(m_device->getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        // recreateSwapChain();
        throw std::runtime_error("OUT OF DATE KHR");
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


void Application::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    //-- renderer begin renderpass
    m_renderer->beginRenderPass(currentFrame, imageIndex);
    //--


    m_pipeline->bind(commandBuffer);
    
    VkDescriptorSet currentDset = m_dSets[currentFrame]->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 0, 1, &currentDset, 0, nullptr);

    m_model->draw(commandBuffer);
    // VkBuffer vertexBuffers[] = { m_vertexBuffer->getVkBuffer() };
    // VkDeviceSize offsets[] = { 0 };
    // vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    // vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);
    // vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    //-- renderer end renderpass
    m_renderer->endRenderPass(currentFrame);
    //--
}

void Application::updateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
    ubo.view = m_camera->getView();
    ubo.proj = m_camera->getProjection();
    
    memcpy(ubos[currentImage]->getMapped(), &ubo, sizeof(ubo));
}

}