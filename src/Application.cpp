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

#include <glm/gtc/matrix_transform.hpp>

bool enableValidationLayers = true;

int cntr = 0;

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
    m_renderer = std::make_shared<Renderer>(m_device, m_window);

    glm::vec2 swapExtent((float)m_renderer->getSwapChain()->getExtent().width,
        (float)m_renderer->getSwapChain()->getExtent().height);

    m_camera = std::make_shared<Camera>(swapExtent, glm::vec3(2.f, 2.f, 2.f));

    ubos.resize(MAX_FRAMES_IN_FLIGHT);
    sbos.resize(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        ubos[i] = std::make_unique<Buffer>(m_device, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        ubos[i]->map();

        sbos[i] = std::make_unique<Buffer>(m_device, sizeof(MeshShaderDataVertex) * MAX_SBOS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        sbos[i]->map();
    }

    m_model = vke::utils::importModel("../res/models/basicPlane/plane.obj");
    auto& textureMap = m_renderer->getTextureMap();
    m_model->afterImportInit(m_device, textureMap);

    // UBO and SSBO set
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding sboLayoutBinding{};
    sboLayoutBinding.binding = 1;
    sboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    sboLayoutBinding.descriptorCount = 1;
    sboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    sboLayoutBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> layoutB;
    layoutB.push_back(uboLayoutBinding);
    layoutB.push_back(sboLayoutBinding);

    std::shared_ptr<DescriptorSetLayout> m_dSetLayout = std::make_shared<DescriptorSetLayout>(m_device, layoutB);

    VkDescriptorPoolSize poolSizeUbo{};
    poolSizeUbo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizeUbo.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolSize poolSizeSbo{};
    poolSizeSbo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizeSbo.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    std::vector<VkDescriptorPoolSize> poolS;
    poolS.push_back(poolSizeUbo);
    poolS.push_back(poolSizeSbo);
    std::shared_ptr<DescriptorPool> m_dPool = std::make_shared<DescriptorPool>(m_device, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), 0, poolS);

    //--

    VkDescriptorSetLayoutBinding bindlessTextLayoutBinding{};
    bindlessTextLayoutBinding.binding = 1;
    bindlessTextLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindlessTextLayoutBinding.descriptorCount = MAX_BINDLESS_RESOURCES;
    bindlessTextLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindlessTextLayoutBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> bindlessLayoutB;
    bindlessLayoutB.push_back(bindlessTextLayoutBinding);

    VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
    
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layoutNext{};
    layoutNext.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    layoutNext.bindingCount = bindlessLayoutB.size();
    layoutNext.pBindingFlags = &bindingFlags;
    layoutNext.pNext = nullptr;

    std::shared_ptr<DescriptorSetLayout> m_bindlessSetLayout = std::make_shared<DescriptorSetLayout>(m_device, bindlessLayoutB,
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, &layoutNext);

    VkDescriptorPoolSize poolSizeBindlessText{};
    poolSizeBindlessText.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizeBindlessText.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_BINDLESS_RESOURCES);

    std::vector<VkDescriptorPoolSize> bindlessPoolS;
    bindlessPoolS.push_back(poolSizeBindlessText);

    std::shared_ptr<DescriptorPool> m_bindlessPool = std::make_shared<DescriptorPool>(m_device,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_BINDLESS_RESOURCES), VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        bindlessPoolS);
    //--

    std::vector<VkDescriptorSetLayout> dSetLayouts = {
        m_dSetLayout->getLayout(),
        m_bindlessSetLayout->getLayout()
        // m_textSetLayout->getLayout()
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = dSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = dSetLayouts.data();

    if (vkCreatePipelineLayout(m_device->getVkDevice(), &pipelineLayoutInfo, nullptr, &m_vkPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    m_pipeline = std::make_shared<Pipeline>(m_device, m_renderer->getSwapChain()->getRenderPass(), "../res/shaders/vert.spv",
        "../res/shaders/frag.spv", m_vkPipelineLayout);

    
    m_dSets.resize(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_dSets[i] = std::make_shared<DescriptorSet>(m_device, m_dSetLayout, m_dPool);

        std::vector<VkDescriptorBufferInfo> bufferInfos{
            ubos[i]->getInfo(),
            sbos[i]->getInfo()
        };

        std::vector<uint32_t> bufferBinding
        {
            0, 1
        };

        m_dSets[i]->addBuffers(bufferBinding, bufferInfos);
    }

    std::shared_ptr<Sampler> m_sampler = std::make_shared<Sampler>(m_device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_MIPMAP_MODE_LINEAR);

    int width, height, channels;
    unsigned char* pixels = utils::loadImage("../res/textures/pepe.jpg", width, height, channels);

    std::shared_ptr<Texture> m_texture = std::make_shared<Texture>(m_device, pixels, glm::vec2(width, height), channels,
        VK_FORMAT_R8G8B8_SRGB);
    m_texture->setSampler(m_sampler);

    m_textures.push_back(m_texture);

    std::shared_ptr<Sampler> m_sampler2 = std::make_shared<Sampler>(m_device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_MIPMAP_MODE_LINEAR);

    pixels = utils::loadImage("../res/textures/negus.jpg", width, height, channels);

    std::shared_ptr<Texture> m_texture2 = std::make_shared<Texture>(m_device, pixels, glm::vec2(width, height), channels,
        VK_FORMAT_R8G8B8_SRGB);
    m_texture2->setSampler(m_sampler2);

    m_textures.push_back(m_texture2);

    m_bindlessSets.resize(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        uint32_t maxBinding = static_cast<uint32_t>(MAX_BINDLESS_RESOURCES - 1);

        VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo{};
        countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
        countInfo.descriptorSetCount = 1;
        countInfo.pDescriptorCounts = &maxBinding;  

        m_bindlessSets[i] = std::make_shared<DescriptorSet>(m_device, m_bindlessSetLayout, m_bindlessPool, &countInfo);

        for (int j = 0; j < m_textures.size(); j++)
        {
            std::vector<VkDescriptorImageInfo> imageInfos{
                m_textures[j]->getInfo()
            };

            std::vector<uint32_t> imageBinding{
                1
            };

            m_bindlessSets[i]->addImages(imageBinding, imageInfos, j);
        }
    }

    free(pixels);
}

void Application::draw()
{
    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
        vke::utils::consumeDeviceInput(m_window->getWindow(), m_camera);
        renderFrame();
        glfwPollEvents();
    }

    vkDeviceWaitIdle(m_device->getVkDevice());
}

void Application::renderFrame()
{
    m_camera->reconstructMatrices();
    
    VkCommandBuffer currentCBuffer = m_renderer->getCommandBuffer(m_currentFrame);

    VkFence currentFence = m_renderer->getSwapChain()->getFenceId(m_currentFrame);
    vkWaitForFences(m_device->getVkDevice(), 1, &currentFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkSemaphore currentImageAvailableSemaphore = m_renderer->getSwapChain()->getImageAvailableSemaphore(m_currentFrame);
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

    updateUniformBuffer(m_currentFrame);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemamphores[] = {currentImageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemamphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentCBuffer;

    VkSemaphore currentRenderFinishedSemaphore = m_renderer->getSwapChain()->getRenderFinishedSemaphore(m_currentFrame);
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

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


void Application::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    //-- renderer begin renderpass
    m_renderer->beginRenderPass(m_currentFrame, imageIndex);
    //--

    m_pipeline->bind(commandBuffer);
    
    VkDescriptorSet currentDset = m_dSets[m_currentFrame]->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 0, 1, &currentDset, 0, nullptr);

    // VkDescriptorSet textDset;
    // if ((cntr / 5000) % 2 == 0)
    //     textDset = m_textSets[m_currentFrame]->getDescriptorSet();
    // else
    //     textDset = m_textSets2[m_currentFrame]->getDescriptorSet();
    // vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 1, 1, &textDset, 0, nullptr);
    VkDescriptorSet bindlessSet = m_bindlessSets[m_currentFrame]->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 1, 1, &bindlessSet, 0, nullptr);

    m_model->draw(commandBuffer);

    //-- renderer end renderpass
    m_renderer->endRenderPass(m_currentFrame);
    //--

    cntr++;
}

void Application::updateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.view = m_camera->getView();
    ubo.proj = m_camera->getProjection();
    
    memcpy(ubos[currentImage]->getMapped(), &ubo, sizeof(ubo));

    std::vector<MeshShaderDataVertex> sboData(1);
    
    sboData[0].model = glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));

    memcpy(sbos[currentImage]->getMapped(), sboData.data(), sizeof(MeshShaderDataVertex) * sboData.size());
}

}