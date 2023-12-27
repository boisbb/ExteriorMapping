#include "Renderer.h"
#include "Model.h"
#include "Scene.h"
#include "Camera.h"
#include "View.h"
#include "Pipeline.h"
#include "pipelines/GraphicsPipeline.h"
#include "pipelines/ComputePipeline.h"
#include "descriptors/SetLayout.h"
#include "descriptors/Pool.h"
#include "descriptors/Set.h"
#include "utils/Constants.h"

#include <cstring>
#include <chrono>

#include <glm/gtc/matrix_transform.hpp>

namespace vke
{

Renderer::Renderer(std::shared_ptr<Device> device, std::shared_ptr<Window> window, std::string vertexShaderFile,
        std::string fragmentShaderFile, std::string computeShaderFile, std::string quadVertexShaderFile,
        std::string quadFragmentShaderFile)
    : m_device(device), m_window(window), m_currentFrame(0), m_fubos(MAX_FRAMES_IN_FLIGHT),
    m_vssbos(MAX_FRAMES_IN_FLIGHT), m_fssbos(MAX_FRAMES_IN_FLIGHT), m_cssbos(MAX_FRAMES_IN_FLIGHT),
    m_generalDescriptorSets(MAX_FRAMES_IN_FLIGHT), m_materialDescriptorSets(MAX_FRAMES_IN_FLIGHT),
    m_computeDescriptorSets(MAX_FRAMES_IN_FLIGHT), m_sceneFramesUpdated(0), m_lightsFramesUpdated(0)
{
    m_swapChain = std::make_shared<SwapChain>(m_device, m_window->getExtent());
    createCommandBuffers();
    createComputeCommandBuffers();
    createDescriptors();
    createPipeline(vertexShaderFile, fragmentShaderFile, computeShaderFile, quadVertexShaderFile, quadFragmentShaderFile);
}

Renderer::~Renderer()
{
}

void Renderer::initDescriptorResources()
{
    // General mesh data
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_generalDescriptorSets[i] = std::make_shared<DescriptorSet>(m_device, m_descriptorSetLayout,
            m_descriptorPool);

        std::vector<VkDescriptorBufferInfo> bufferInfos{
            m_vssbos[i]->getInfo(),
            m_fubos[i]->getInfo(),
            m_fssbos[i]->getInfo()
        };

        std::vector<uint32_t> bufferBinding
        {
            1, 2, 3
        };

        m_generalDescriptorSets[i]->addBuffers(bufferBinding, bufferInfos);

        std::vector<VkDescriptorImageInfo> imageInfos{
            m_swapChain->getOffscreenImageInfo(),
        };

        std::vector<uint32_t> imageBinding{
            4
        };

        m_generalDescriptorSets[i]->addImages(imageBinding, imageInfos, 0);
    }

    // Materials
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        uint32_t maxBinding = static_cast<uint32_t>(MAX_BINDLESS_RESOURCES - 1);

        VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo{};
        countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
        countInfo.descriptorSetCount = 1;
        countInfo.pDescriptorCounts = &maxBinding;  

        m_materialDescriptorSets[i] = std::make_shared<DescriptorSet>(m_device, m_materialSetLayout, m_materialPool, &countInfo);

        for (int j = 0; j < m_textures.size(); j++)
        {
            std::vector<VkDescriptorImageInfo> imageInfos{
                m_textures[j]->getInfo()
            };

            std::vector<uint32_t> imageBinding{
                0
            };

            m_materialDescriptorSets[i]->addImages(imageBinding, imageInfos, j);
        }

        for (int j = 0; j < m_bumpTextures.size(); j++)
        {
            std::vector<VkDescriptorImageInfo> imageInfos{
                m_bumpTextures[j]->getInfo()
            };

            std::vector<uint32_t> imageBinding{
                1
            };

            m_materialDescriptorSets[i]->addImages(imageBinding, imageInfos, j);
        }
    }

    // Compute
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_computeDescriptorSets[i] = std::make_shared<DescriptorSet>(m_device, m_computeSetLayout,
            m_computePool);

        std::vector<VkDescriptorBufferInfo> bufferInfos = {
            m_cssbos[i]->getInfo()
        };

        std::vector<uint32_t> bufferBinding = {
            1
        };


        m_computeDescriptorSets[i]->addBuffers(bufferBinding, bufferInfos);
    }
}

void Renderer::computePass(const std::shared_ptr<Scene> &scene, const std::vector<std::shared_ptr<View>>& views)
{
    // Compute part
    VkFence currentComputeFence = m_swapChain->getComputeFenceId(m_currentFrame);
    vkWaitForFences(m_device->getVkDevice(), 1, &currentComputeFence, VK_TRUE, UINT64_MAX);

    vkResetFences(m_device->getVkDevice(), 1, &currentComputeFence);

    updateComputeDescriptorData(scene);
    vkResetCommandBuffer(m_computeCommandBuffers[m_currentFrame], 0);

     VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_computeCommandBuffers[m_currentFrame], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin compute command buffer");

    for (auto& view : views)
    {
        view->getCamera()->reconstructMatrices();
        view->updateComputeDescriptorData(m_currentFrame, scene);

        if (!scene->viewResourcesExist(view))
        {
            scene->createViewResources(view, m_device, m_computeSceneSetLayout, m_computeScenePool);

            scene->setReinitializeDebugCameraGeometryFlag(true);
            if (scene->getRenderDebugGeometryFlag())
            {
                scene->addDebugCameraGeometry(views);
            }
        }

        recordComputeCommandBuffer(m_computeCommandBuffers[m_currentFrame], scene, view);
    }

    if (vkEndCommandBuffer(m_computeCommandBuffers[m_currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("failed to end compute command buffer");

    VkSemaphore currentComputeFinishedSemaphore = m_swapChain->getComputeFinishedSemaphore(m_currentFrame);

    VkSubmitInfo computeSubmitInfo{};
    computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    computeSubmitInfo.commandBufferCount = 1;
    computeSubmitInfo.pCommandBuffers = &m_computeCommandBuffers[m_currentFrame];
    computeSubmitInfo.signalSemaphoreCount = 1;
    computeSubmitInfo.pSignalSemaphores = &currentComputeFinishedSemaphore;

    VkResult res = vkQueueSubmit(m_device->getComputeQueue(), 1, &computeSubmitInfo, currentComputeFence);
    if (res != VK_SUCCESS)
        throw std::runtime_error("failed to submit compute command buffer");
}

void Renderer::renderPass(const std::shared_ptr<Scene> &scene, const std::vector<std::shared_ptr<View>> views)
{
    bool resizeViews = false;

    // uint32_t imageIndex = prepareFrame(scene, nullptr, m_window, resizeViews);

    // beginRenderPass(glm::vec2(WIDTH, HEIGHT), m_currentFrame, imageIndex);

    updateDescriptorData(scene, views);

    for (auto& view : views)
    {
        view->getCamera()->reconstructMatrices();
        view->updateDescriptorData(m_currentFrame);

        glm::vec2 viewportStart = view->getViewportStart();
        glm::vec2 viewResolution = view->getResolution();

        setViewport(viewportStart, viewResolution);
        setScissor(viewportStart, viewResolution);

        recordCommandBuffer(m_commandBuffers[m_currentFrame], scene, view);
    }

    // endRenderPass(m_currentFrame);
}

void Renderer::quadRenderPass(glm::vec2 windowResolution)
{
    setViewport(glm::vec2(0, 0), windowResolution);
    setScissor(glm::vec2(0, 0), windowResolution);

    m_quadPipeline->bind(m_commandBuffers[m_currentFrame]);

    VkDescriptorSet descriptorSet = m_generalDescriptorSets[m_currentFrame]->getDescriptorSet();
    vkCmdBindDescriptorSets(m_commandBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_quadPipeline->getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

    vkCmdDraw(m_commandBuffers[m_currentFrame], 3, 1, 0, 0);
}

void Renderer::setViewport(const glm::vec2& viewportStart, const glm::vec2& viewportResolution)
{
    VkViewport viewport{};
    viewport.x = viewportStart.x;
    viewport.y = viewportStart.y;
    viewport.width = viewportResolution.x;
    viewport.height = viewportResolution.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffers[m_currentFrame], 0, 1, &viewport);
}

void Renderer::setScissor(const glm::vec2& viewportStart, const glm::vec2& viewportResolution)
{
    VkRect2D scissor{};
    scissor.offset = { 0,0 };
    scissor.extent = m_swapChain->getExtent();
    vkCmdSetScissor(m_commandBuffers[m_currentFrame], 0, 1, &scissor);
}

uint32_t Renderer::prepareFrame(const std::shared_ptr<Scene>& scene, std::shared_ptr<View> view,
    std::shared_ptr<Window> window, bool& resizeViews)
{
    // Graphics part
    VkFence currentFence = m_swapChain->getFenceId(m_currentFrame);
    vkWaitForFences(m_device->getVkDevice(), 1, &currentFence, VK_TRUE, UINT64_MAX);
    //updateDescriptorData(scene, camera);

    uint32_t imageIndex;
    VkSemaphore currentImageAvailableSemaphore = m_swapChain->getImageAvailableSemaphore(m_currentFrame);
    VkResult result = vkAcquireNextImageKHR(m_device->getVkDevice(), m_swapChain->getSwapChain(),
        UINT64_MAX, currentImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        VkExtent2D ext = window->getExtent();
        m_swapChain->recreate(ext);

        std::cout << "out of date" << std::endl;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Error: failed to acquire swap chain image.");
    }

    vkResetFences(m_device->getVkDevice(), 1, &currentFence);

    VkCommandBuffer currentCommandBuffer = m_commandBuffers[m_currentFrame];

    vkResetCommandBuffer(currentCommandBuffer, 0);

    return imageIndex;
}

void Renderer::submitFrame()
{
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];

    VkCommandBuffer currentCommandBuffer = commandBuffer;
    VkSemaphore currentImageAvailableSemaphore = m_swapChain->getImageAvailableSemaphore(m_currentFrame);
    VkFence currentFence = m_swapChain->getFenceId(m_currentFrame);
    
    VkSemaphore currentComputeFinishedSemaphore = m_swapChain->getComputeFinishedSemaphore(m_currentFrame);
    
    VkSemaphore waitSemamphores[] = { currentComputeFinishedSemaphore,
        currentImageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 2;
    submitInfo.pWaitSemaphores = waitSemamphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentCommandBuffer;

    VkSemaphore currentRenderFinishedSemaphore = m_swapChain->getRenderFinishedSemaphore(m_currentFrame);
    VkSemaphore signalSemaphores[] = {currentRenderFinishedSemaphore};

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkResult res = vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, currentFence);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

void Renderer::presentFrame(const uint32_t& imageIndex, std::shared_ptr<Window> window, std::shared_ptr<View> view,
    bool& resizeViews)
{
    // std::shared_ptr<Camera> camera = view->getCamera();

    VkSemaphore currentRenderFinishedSemaphore = m_swapChain->getRenderFinishedSemaphore(m_currentFrame);
    VkSemaphore signalSemaphores[] = {currentRenderFinishedSemaphore};

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapChain->getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    VkResult result = vkQueuePresentKHR(m_device->getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->resized())
    {
        VkExtent2D ext = window->getExtent();
        m_swapChain->recreate(ext);

        m_window->setResized(false);
        resizeViews = true;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image");
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


std::shared_ptr<SwapChain> Renderer::getSwapChain() const
{
    return m_swapChain;
}

VkCommandBuffer Renderer::getCommandBuffer(int id) const
{
    return m_commandBuffers[id];
}

std::unordered_map<std::string, int>& Renderer::getTextureMap()
{
    return m_textureMap;
}

int Renderer::getTextureId(std::string fileName)
{
    if (m_textureMap.find(fileName) != m_textureMap.end())
        return m_textureMap[fileName];
    else
        return RET_ID_NOT_FOUND;
}

std::vector<std::shared_ptr<Texture>> Renderer::getTextures() const
{
    return m_textures;
}

int Renderer::getBumpTextureId(std::string fileName)
{
    if (m_bumpTextureMap.find(fileName) != m_bumpTextureMap.end())
        return m_bumpTextureMap[fileName];
    else
        return RET_ID_NOT_FOUND;
}

std::vector<std::shared_ptr<Texture>> Renderer::getBumpTextures() const
{
    return m_bumpTextures;
}

int Renderer::getCurrentFrame() const
{
    return m_currentFrame;
}

VkCommandBuffer Renderer::getCurrentCommandBuffer() const
{
    return m_commandBuffers[m_currentFrame];
}

std::shared_ptr<DescriptorSetLayout> Renderer::getDescriptorSetLayout() const
{
    return m_descriptorSetLayout;
}

std::shared_ptr<DescriptorPool> Renderer::getDescriptorPool() const
{
    return m_descriptorPool;
}

std::shared_ptr<DescriptorSetLayout> Renderer::getComputeDescriptorSetLayout() const
{
    return m_computeSetLayout;
}

std::shared_ptr<DescriptorPool> Renderer::getComputeDescriptorPool() const
{
    return m_computePool;
}

std::shared_ptr<DescriptorSetLayout> Renderer::getSceneComputeDescriptorSetLayout() const
{
    return m_computeSceneSetLayout;
}

std::shared_ptr<DescriptorPool> Renderer::getSceneComputeDescriptorPool() const
{
    return m_computeScenePool;
}

std::shared_ptr<DescriptorSetLayout> Renderer::getViewDescriptorSetLayout() const
{
    return m_viewSetLayout;
}

std::shared_ptr<DescriptorPool> Renderer::getViewDescriptorPool() const
{
    return m_viewPool;
}

VkRenderPass Renderer::getOffscreenRenderPass() const
{
    return m_graphicsPipeline->getRenderPass();
}

VkRenderPass Renderer::getQuadRenderPass() const
{
return m_quadPipeline->getRenderPass();
}

VkFramebuffer Renderer::getOffscreenFramebuffer() const
{
    return m_swapChain->getOffscreenFramebuffer();
}

VkFramebuffer Renderer::getQuadFramebuffer(int imageIndex)
{
    return m_swapChain->getFramebuffer(imageIndex);
}

int Renderer::addTexture(std::shared_ptr<Texture> texture, std::string filename)
{
    m_textures.push_back(texture);
    m_textureMap[filename] = m_textures.size() - 1;

    return m_textures.size() - 1;
}

int Renderer::addBumpTexture(std::shared_ptr<Texture> texture, std::string filename)
{
    m_bumpTextures.push_back(texture);
    m_bumpTextureMap[filename] = m_bumpTextures.size() - 1;

    return m_bumpTextures.size() - 1;
}

void Renderer::beginCommandBuffer()
{
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
}

void Renderer::beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, glm::vec2 windowResolution,
    glm::uvec2 renderArea, bool clear)
{
    VkExtent2D swapChainExtent = m_swapChain->getExtent();

    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    renderPassInfo.renderPass = renderPass;

    if (windowResolution.x > swapChainExtent.width)
        windowResolution.x = swapChainExtent.width;
    
    if (windowResolution.y > swapChainExtent.height)
        windowResolution.y = swapChainExtent.height;

    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {renderArea.x, renderArea.y};

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.f, 0.f, 0.f, 1.f} };
    clearValues[1].depthStencil = { 1.f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::endRenderPass()
{
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];

    vkCmdEndRenderPass(commandBuffer);
}

void Renderer::endCommandBuffer()
{
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void Renderer::createCommandBuffers()
{
    m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_device->getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

    if (vkAllocateCommandBuffers(m_device->getVkDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    m_commandBuffers2.resize(MAX_FRAMES_IN_FLIGHT);

    //VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_device->getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_commandBuffers2.size();

    if (vkAllocateCommandBuffers(m_device->getVkDevice(), &allocInfo, m_commandBuffers2.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Renderer::createComputeCommandBuffers()
{
    m_computeCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo computeAllocInfo{};
    computeAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    computeAllocInfo.commandPool = m_device->getCommandPool();
    computeAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    computeAllocInfo.commandBufferCount = (uint32_t)m_computeCommandBuffers.size();

    if (vkAllocateCommandBuffers(m_device->getVkDevice(), &computeAllocInfo, m_computeCommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate compute command buffers!");
    }
}

void Renderer::createDescriptors()
{
    // General mesh data
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_fubos[i] = std::make_unique<Buffer>(m_device, sizeof(UniformDataFragment),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_fubos[i]->map();

        m_vssbos[i] = std::make_unique<Buffer>(m_device, sizeof(MeshShaderDataVertex) * MAX_SBOS,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_vssbos[i]->map();

        m_fssbos[i] = std::make_unique<Buffer>(m_device, sizeof(MeshShaderDataFragment) * MAX_SBOS,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_fssbos[i]->map();
    }

    VkDescriptorSetLayoutBinding vssboLayoutBinding = createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1, VK_SHADER_STAGE_VERTEX_BIT);
    VkDescriptorSetLayoutBinding fuboLayoutBinding = createDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkDescriptorSetLayoutBinding fssboLayoutBinding = createDescriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkDescriptorSetLayoutBinding finalImageLayoutBinding = createDescriptorSetLayoutBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        1, VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {
        vssboLayoutBinding,
        fuboLayoutBinding,
        fssboLayoutBinding,
        finalImageLayoutBinding
    };

    m_descriptorSetLayout = std::make_shared<DescriptorSetLayout>(m_device, layoutBindings);

    VkDescriptorPoolSize vssboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_SBOS));
    VkDescriptorPoolSize fuboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    VkDescriptorPoolSize fssboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_SBOS));
    VkDescriptorPoolSize finalImagePoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

    std::vector<VkDescriptorPoolSize> poolSizes = {
        vssboPoolSize,
        fuboPoolSize,
        fssboPoolSize,
        finalImagePoolSize
    };
    m_descriptorPool = std::make_shared<DescriptorPool>(m_device,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), 0, poolSizes);

    //! View Data
    VkDescriptorSetLayoutBinding viewLayoutBinding = createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1, VK_SHADER_STAGE_VERTEX_BIT);
    VkDescriptorSetLayoutBinding cuboLayoutBinding = createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1, VK_SHADER_STAGE_COMPUTE_BIT);
    VkDescriptorSetLayoutBinding fvuboLayoutBinding = createDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1, VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<VkDescriptorSetLayoutBinding> viewLayoutBindings = {
        viewLayoutBinding,
        cuboLayoutBinding,
        fvuboLayoutBinding
    };

    m_viewSetLayout = std::make_shared<DescriptorSetLayout>(m_device, viewLayoutBindings);

    VkDescriptorPoolSize viewPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_VIEWS));
    VkDescriptorPoolSize cuboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_VIEWS));
    VkDescriptorPoolSize fvuboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_VIEWS));
    
    std::vector<VkDescriptorPoolSize> viewSizes = {
        viewPoolSize,
        cuboPoolSize,
        fvuboPoolSize
    };

    m_viewPool = std::make_shared<DescriptorPool>(m_device, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_VIEWS), 
        0, viewSizes);

    //! Textures
    VkDescriptorSetLayoutBinding textureLayoutBinding = createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        MAX_BINDLESS_RESOURCES, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding bumpLayoutBinding = createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        MAX_BINDLESS_RESOURCES, VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<VkDescriptorSetLayoutBinding> materialLayoutBindings = {
        textureLayoutBinding,
        bumpLayoutBinding
    };

    VkDescriptorBindingFlags textureBindingFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

    VkDescriptorBindingFlags bumpBindingFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

    std::vector<VkDescriptorBindingFlags> materialBindingFlags = {
        textureBindingFlags,
        bumpBindingFlags
    };

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layoutNext{};
    layoutNext.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    layoutNext.bindingCount = materialLayoutBindings.size();
    layoutNext.pBindingFlags = materialBindingFlags.data();
    layoutNext.pNext = nullptr;

    m_materialSetLayout = std::make_shared<DescriptorSetLayout>(m_device, materialLayoutBindings,
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, &layoutNext);
    
    VkDescriptorPoolSize texturePoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_BINDLESS_RESOURCES));

    VkDescriptorPoolSize bumpPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_BINDLESS_RESOURCES));

    std::vector<VkDescriptorPoolSize> materialPoolSizes = {
        texturePoolSize,
        bumpPoolSize
    };

    m_materialPool = std::make_shared<DescriptorPool>(m_device,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_BINDLESS_RESOURCES), VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        materialPoolSizes);

    // Compute general
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_cssbos[i] = std::make_unique<Buffer>(m_device, sizeof(MeshShaderDataCompute) * MAX_SBOS,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_cssbos[i]->map();
    }

    VkDescriptorSetLayoutBinding cssboLayoutBinding = createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> computeGeneralLayoutBindings = {
        // cuboLayoutBinding,
        cssboLayoutBinding
    };

    m_computeSetLayout = std::make_shared<DescriptorSetLayout>(m_device, computeGeneralLayoutBindings);

    VkDescriptorPoolSize cssboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_SBOS));
    
    std::vector<VkDescriptorPoolSize> computeGeneralSizes = {
        cssboPoolSize
    };

    m_computePool = std::make_shared<DescriptorPool>(m_device,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), 0, computeGeneralSizes);

    //  Compute scene
    VkDescriptorSetLayoutBinding drawLayoutBinding = createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> computeSceneLayoutBindings = {
        drawLayoutBinding
    };

    m_computeSceneSetLayout = std::make_shared<DescriptorSetLayout>(m_device, computeSceneLayoutBindings);

    VkDescriptorPoolSize drawPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_SBOS));
    
    std::vector<VkDescriptorPoolSize> computeSceneSizes = {
        drawPoolSize
    };
    m_computeScenePool = std::make_shared<DescriptorPool>(m_device, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 
        static_cast<uint32_t>(MAX_VIEWS), 0, computeSceneSizes);

}

void Renderer::createPipeline(std::string vertexShaderFile, std::string fragmentShaderFile,
    std::string computeShaderFile, std::string quadVertexShaderFile, std::string quadFragmentShaderFile)
{
    std::vector<VkDescriptorSetLayout> offscreenGraphicsSetLayouts = {
        m_descriptorSetLayout->getLayout(),
        m_materialSetLayout->getLayout(),
        m_viewSetLayout->getLayout()
    };

    std::vector<VkDescriptorSetLayout> computeSetLayouts = {
        m_computeSetLayout->getLayout(),
        m_computeSceneSetLayout->getLayout(),
        m_viewSetLayout->getLayout()
    };

    std::vector<VkDescriptorSetLayout> quadSetLayout = {
        m_descriptorSetLayout->getLayout()
    };

    m_graphicsPipeline = std::make_shared<GraphicsPipeline>(m_device, m_swapChain->getOffscreenRenderPass(), vertexShaderFile,
        fragmentShaderFile, offscreenGraphicsSetLayouts);

    m_quadPipeline = std::make_shared<GraphicsPipeline>(m_device, m_swapChain->getRenderPass(), quadVertexShaderFile,
        quadFragmentShaderFile, quadSetLayout, false, false);
    
    m_computePipeline = std::make_shared<ComputePipeline>(m_device, computeShaderFile, computeSetLayouts);
}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<Scene>& scene,
    const std::shared_ptr<View>& view)
{
    m_graphicsPipeline->bind(commandBuffer);

    VkDescriptorSet descriptorSet = m_generalDescriptorSets[m_currentFrame]->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline->getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

    VkDescriptorSet textureSet = m_materialDescriptorSets[m_currentFrame]->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline->getPipelineLayout(), 1, 1, &textureSet, 0, nullptr);

    VkDescriptorSet viewSet = view->getViewDescriptorSet(m_currentFrame)->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline->getPipelineLayout(), 2, 1, &viewSet, 0, nullptr);

    scene->draw(view, commandBuffer, m_currentFrame);
}

void Renderer::recordComputeCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<Scene>& scene,
    const std::shared_ptr<View>& view)
{
    VkDescriptorSet computeSet = m_computeDescriptorSets[m_currentFrame]->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline->getPipelineLayout(), 0, 1, &computeSet, 0, nullptr);

    VkDescriptorSet viewSet = view->getViewDescriptorSet(m_currentFrame)->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline->getPipelineLayout(), 2, 1, &viewSet, 0, nullptr);

    m_computePipeline->bind(commandBuffer);

    scene->dispatch(view, commandBuffer, m_computePipeline->getPipelineLayout(), m_currentFrame);
}

void Renderer::updateDescriptorData(const std::shared_ptr<Scene>& scene, const std::vector<std::shared_ptr<View>>& views)
{
    if (scene->lightChanged())
    {
        UniformDataFragment fubo{};
        fubo.lightPos = scene->getLightPos();
        m_fubos[m_currentFrame]->copyMapped(&fubo, sizeof(UniformDataFragment));

        m_lightsFramesUpdated++;

        if (m_lightsFramesUpdated == MAX_FRAMES_IN_FLIGHT)
            scene->setLightChanged(false);
    }

    if (scene->sceneChanged() || scene->getRenderDebugGeometryFlag())
    {
        std::vector<MeshShaderDataVertex> vssboData;
        std::vector<MeshShaderDataFragment> fssboData;

        std::vector<std::shared_ptr<Model>>& models = scene->getModels();

        for (auto& model : models)
            model->updateDescriptorData(vssboData, fssboData);

        for (auto& model : models)
            model->updateDescriptorData(vssboData, fssboData, true);

        if (scene->getRenderDebugGeometryFlag())
        {
            // std::cout << "update descriptor data for views" << std::endl;
            for (auto& view : views)
            {
                view->updateDescriptorDataRenderDebugCube(vssboData, fssboData);
            }
        }

        m_vssbos[m_currentFrame]->copyMapped(vssboData.data(), sizeof(MeshShaderDataVertex) * vssboData.size());
        m_fssbos[m_currentFrame]->copyMapped(fssboData.data(), sizeof(MeshShaderDataFragment) * fssboData.size());

        m_sceneFramesUpdated++;

        if (m_sceneFramesUpdated == MAX_FRAMES_IN_FLIGHT)
            scene->setSceneChanged(false);
    }
}

void Renderer::updateComputeDescriptorData(const std::shared_ptr<Scene> &scene)
{
    if (scene->sceneChanged())
    {
        std::vector<MeshShaderDataCompute> cssboData;

        std::vector<std::shared_ptr<Model>>& models = scene->getModels();

        for (auto& model : models)
            model->updateComputeDescriptorData(cssboData);

        for (auto& model : models)
            model->updateComputeDescriptorData(cssboData, true);

        m_cssbos[m_currentFrame]->copyMapped(cssboData.data(), sizeof(MeshShaderDataCompute) * cssboData.size());
    }
}
}