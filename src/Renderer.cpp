#include "Renderer.h"
#include "Model.h"
#include "Scene.h"
#include "Camera.h"
#include "Pipeline.h"
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
        std::string fragmentShaderFile, std::string computeShaderFile)
    : m_device(device), m_window(window), m_currentFrame(0),
    m_vubos(MAX_FRAMES_IN_FLIGHT), m_fubos(MAX_FRAMES_IN_FLIGHT),
    m_vssbos(MAX_FRAMES_IN_FLIGHT), m_fssbos(MAX_FRAMES_IN_FLIGHT),
    m_generalDescriptorSets(MAX_FRAMES_IN_FLIGHT), m_materialDescriptorSets(MAX_FRAMES_IN_FLIGHT)
{
    m_swapChain = std::make_shared<SwapChain>(m_device, m_window->getExtent());
    createCommandBuffers();
    createComputeCommandBuffers();
    createDescriptors();
    createPipeline(vertexShaderFile, fragmentShaderFile, computeShaderFile);
}

Renderer::~Renderer()
{
}

void Renderer::initDescriptorResources()
{
    // Buffers
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_generalDescriptorSets[i] = std::make_shared<DescriptorSet>(m_device, m_descriptorSetLayout,
            m_descriptorPool);

        std::vector<VkDescriptorBufferInfo> bufferInfos{
            m_vubos[i]->getInfo(),
            m_vssbos[i]->getInfo(),
            m_fubos[i]->getInfo(),
            m_fssbos[i]->getInfo()
        };

        std::vector<uint32_t> bufferBinding
        {
            0, 1, 2, 3
        };

        m_generalDescriptorSets[i]->addBuffers(bufferBinding, bufferInfos);
    }

    // Images
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

    // for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    // {
    //     m_indirectDrawBuffers[i] = std::make_unique<Buffer>(device, sizeof(VkDrawIndexedIndirectCommand) * MAX_SBOS,
    //         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    //         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    //     m_indirectDrawBuffers[i]->map();
// 
    //     m_indirectDrawBuffers[i]->copyMapped(m_indirectDrawBuffer->getMapped(), m_indirectDrawBuffer->getSize());
// 
    //     m_computeDescriptorSets[i] = std::make_shared<DescriptorSet>(device, descriptorSetLayout, descriptorPool);
// 
    //     std::vector<VkDescriptorBufferInfo> bufferInfos = {
    //         m_indirectDrawBuffers[i]->getInfo()
    //     };
// 
    //     std::vector<uint32_t> bufferBinding = {
    //         0
    //     };
// 
    //     m_computeDescriptorSets[i]->addBuffers(bufferBinding, bufferInfos);
    // }
}

uint32_t Renderer::prepareFrame(const std::shared_ptr<Scene>& scene, std::shared_ptr<Camera> camera)
{

    // Compute part

    VkFence currentComputeFence = m_swapChain->getComputeFenceId(m_currentFrame);
    vkWaitForFences(m_device->getVkDevice(), 1, &currentComputeFence, VK_TRUE, UINT64_MAX);

    vkResetFences(m_device->getVkDevice(), 1, &currentComputeFence);
    
    VkCommandBuffer currentComputeCommandBuffer = m_computeCommandBuffers[m_currentFrame];

    vkResetCommandBuffer(currentComputeCommandBuffer, 0);

    recordComputeCommandBuffer(currentComputeCommandBuffer, scene);

    VkSemaphore currentComputeFinishedSemaphore = m_swapChain->getComputeFinishedSemaphore(m_currentFrame);

    VkSubmitInfo computeSubmitInfo{};
    computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    computeSubmitInfo.commandBufferCount = 1;
    computeSubmitInfo.pCommandBuffers = &currentComputeCommandBuffer;
    computeSubmitInfo.signalSemaphoreCount = 1;
    computeSubmitInfo.pSignalSemaphores = &currentComputeFinishedSemaphore;

    VkResult res = vkQueueSubmit(m_device->getComputeQueue(), 1, &computeSubmitInfo, currentComputeFence);
    if (res != VK_SUCCESS)
        throw std::runtime_error("failed to submit compute command buffer");

    // Graphics part
    VkFence currentFence = m_swapChain->getFenceId(m_currentFrame);
    vkWaitForFences(m_device->getVkDevice(), 1, &currentFence, VK_TRUE, UINT64_MAX);

    camera->reconstructMatrices();
    updateDescriptorData(scene, camera);
    
    vkResetFences(m_device->getVkDevice(), 1, &currentFence);

    VkCommandBuffer currentCommandBuffer = m_commandBuffers[m_currentFrame];

    vkResetCommandBuffer(currentCommandBuffer, 0);

    uint32_t imageIndex;
    VkSemaphore currentImageAvailableSemaphore = m_swapChain->getImageAvailableSemaphore(m_currentFrame);
    VkResult result = vkAcquireNextImageKHR(m_device->getVkDevice(), m_swapChain->getSwapChain(),
        UINT64_MAX, currentImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // rebuildSwapChain();
        throw std::runtime_error("Error: out of date KHR.");
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Error: failed to acquire swap chain image.");
    }

    beginRenderPass(m_currentFrame, imageIndex);

    return imageIndex;
}

void Renderer::presentFrame(const uint32_t& imageIndex)
{
    endRenderPass(m_currentFrame);

    VkCommandBuffer currentCommandBuffer = m_commandBuffers[m_currentFrame];
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

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        // TODO: also sort out resizing of window
        // recreateSwapChain();
        throw std::runtime_error("OUT OF DATE KHR");
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

void Renderer::beginRenderPass(int currentFrame, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(m_commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_swapChain->getRenderPass();
    renderPassInfo.framebuffer = m_swapChain->getFramebuffer(imageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapChain->getExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.f, 0.f, 0.f, 1.f} };
    clearValues[1].depthStencil = { 1.f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapChain->getExtent().width);
    viewport.height = static_cast<float>(m_swapChain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffers[currentFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChain->getExtent();
    vkCmdSetScissor(m_commandBuffers[currentFrame], 0, 1, &scissor);
}

void Renderer::endRenderPass(int currentFrame)
{
    vkCmdEndRenderPass(m_commandBuffers[currentFrame]);

    if (vkEndCommandBuffer(m_commandBuffers[currentFrame]) != VK_SUCCESS)
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
        m_vubos[i] = std::make_unique<Buffer>(m_device, sizeof(UniformDataVertex),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_vubos[i]->map();

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

    VkDescriptorSetLayoutBinding vuboLayoutBinding = createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1, VK_SHADER_STAGE_VERTEX_BIT);
    VkDescriptorSetLayoutBinding vssboLayoutBinding = createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1, VK_SHADER_STAGE_VERTEX_BIT);
    VkDescriptorSetLayoutBinding fuboLayoutBinding = createDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkDescriptorSetLayoutBinding fssboLayoutBinding = createDescriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1, VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {
        vuboLayoutBinding,
        vssboLayoutBinding,
        fuboLayoutBinding,
        fssboLayoutBinding
    };

    m_descriptorSetLayout = std::make_shared<DescriptorSetLayout>(m_device, layoutBindings);

    VkDescriptorPoolSize vuboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    VkDescriptorPoolSize vssboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_SBOS));
    VkDescriptorPoolSize fuboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    VkDescriptorPoolSize fssboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_SBOS));

    std::vector<VkDescriptorPoolSize> poolSizes = {
        vuboPoolSize,
        vssboPoolSize,
        fuboPoolSize,
        fssboPoolSize
    };
    m_descriptorPool = std::make_shared<DescriptorPool>(m_device,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), 0, poolSizes);

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

}

void Renderer::createPipeline(std::string vertexShaderFile, std::string fragmentShaderFile,
    std::string computeShaderFile)
{
    std::vector<VkDescriptorSetLayout> graphicsSetLayouts = {
        m_descriptorSetLayout->getLayout(),
        m_materialSetLayout->getLayout()
    };

    std::vector<VkDescriptorSetLayout> computeSetLayouts = {
        // m_computeSetLayout->getLayout()
    };

    m_pipeline = std::make_shared<Pipeline>(m_device, m_swapChain->getRenderPass(), vertexShaderFile,
        fragmentShaderFile, computeShaderFile, graphicsSetLayouts, computeSetLayouts);
}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<Scene>& scene,
    uint32_t imageIndex)
{
    m_pipeline->bindGraphics(commandBuffer);

    VkDescriptorSet descriptorSet = m_generalDescriptorSets[m_currentFrame]->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getGraphicsPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

    VkDescriptorSet textureSet = m_materialDescriptorSets[m_currentFrame]->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getGraphicsPipelineLayout(), 1, 1, &textureSet, 0, nullptr);

    scene->draw(commandBuffer, m_currentFrame);
}

void Renderer::recordComputeCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<Scene>& scene)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin compute command buffer");

    m_pipeline->bindCompute(commandBuffer);

    scene->dispatch(commandBuffer, m_pipeline->getComputePipelineLayout(), m_currentFrame);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to end compute command buffer");
}

void Renderer::updateDescriptorData(const std::shared_ptr<Scene>& scene, std::shared_ptr<Camera> camera)
{
    std::vector<std::shared_ptr<Model>>& models = scene->getModels();

    UniformDataVertex vubo{};
    vubo.view = camera->getView();
    vubo.proj = camera->getProjection();
    m_vubos[m_currentFrame]->copyMapped(&vubo, sizeof(UniformDataVertex));

    UniformDataFragment fubo{};
    fubo.lightPos = scene->getLightPos();
    m_fubos[m_currentFrame]->copyMapped(&fubo, sizeof(UniformDataFragment));

    std::vector<MeshShaderDataVertex> vssboData;
    std::vector<MeshShaderDataFragment> fssboData;

    for (auto& model : models)
    {
        model->updateDescriptorData(vssboData, fssboData);
    }

    m_vssbos[m_currentFrame]->copyMapped(vssboData.data(), sizeof(MeshShaderDataVertex) * vssboData.size());
    m_fssbos[m_currentFrame]->copyMapped(fssboData.data(), sizeof(MeshShaderDataFragment) * fssboData.size());

    MeshShaderDataFragment* mapped = (MeshShaderDataFragment*)m_fssbos[m_currentFrame]->getMapped();
    auto  a = mapped[0];

    std::cout << std::endl;
    // glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
}

}