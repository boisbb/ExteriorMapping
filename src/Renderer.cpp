/**
 * @file Renderer.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

// vke
#include "Renderer.h"
#include "Model.h"
#include "Scene.h"
#include "Camera.h"
#include "View.h"
#include "ViewGrid.h"
#include "Image.h"
#include "Sampler.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "pipelines/GraphicsPipeline.h"
#include "pipelines/ComputePipeline.h"
#include "descriptors/SetLayout.h"
#include "descriptors/Pool.h"
#include "descriptors/Set.h"
#include "utils/Constants.h"

#include <glm/gtc/matrix_transform.hpp>

// #define RAY_EVAL_DEBUG

#define INTERPOLATE_PIXELS_X 1.f
#define INTERPOLATE_PIXELS_Y 1.f

namespace vke
{

Renderer::Renderer(std::shared_ptr<Device> device, std::shared_ptr<Window> window, const RendererInitParams& params)
    : m_device(device), m_window(window), m_currentFrame(0), m_fubos(MAX_FRAMES_IN_FLIGHT),
    m_vssbos(MAX_FRAMES_IN_FLIGHT), m_fssbos(MAX_FRAMES_IN_FLIGHT), m_cssbos(MAX_FRAMES_IN_FLIGHT),
    m_creubo(MAX_FRAMES_IN_FLIGHT), m_cressbo(MAX_FRAMES_IN_FLIGHT), m_creDebugSsbo(MAX_FRAMES_IN_FLIGHT), m_creHitsssbo(MAX_FRAMES_IN_FLIGHT),
    m_creHitsCntssbo(MAX_FRAMES_IN_FLIGHT), m_quadubo(MAX_FRAMES_IN_FLIGHT),
    m_generalDescriptorSets(MAX_FRAMES_IN_FLIGHT), m_materialDescriptorSets(MAX_FRAMES_IN_FLIGHT),
    m_computeDescriptorSets(MAX_FRAMES_IN_FLIGHT), m_computeRayEvalDescriptorSets(MAX_FRAMES_IN_FLIGHT),
    m_quadDescriptorSets(MAX_FRAMES_IN_FLIGHT), m_sceneFramesUpdated(0), m_lightsFramesUpdated(0),
    m_swapChainImageIndices(MAX_FRAMES_IN_FLIGHT), m_secondarySwapchain(nullptr), m_secondaryQuadubo(MAX_FRAMES_IN_FLIGHT),
    m_secondaryQuadDescriptorSets(MAX_FRAMES_IN_FLIGHT), m_pointCloudDescriptorsets(MAX_FRAMES_IN_FLIGHT),
    m_pointCloudUbo(MAX_FRAMES_IN_FLIGHT), m_pointClouds(MAX_FRAMES_IN_FLIGHT)
{
    createCommandBuffers();
    createComputeCommandBuffers();
    createDescriptors();
    createRenderResources();
    createPipeline(params);
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

        m_generalDescriptorSets[i]->updateBuffers(bufferBinding, bufferInfos);
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

            m_materialDescriptorSets[i]->updateImages(imageBinding, imageInfos, j);
        }

        for (int j = 0; j < m_bumpTextures.size(); j++)
        {
            std::vector<VkDescriptorImageInfo> imageInfos{
                m_bumpTextures[j]->getInfo()
            };

            std::vector<uint32_t> imageBinding{
                1
            };

            m_materialDescriptorSets[i]->updateImages(imageBinding, imageInfos, j);
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


        m_computeDescriptorSets[i]->updateBuffers(bufferBinding, bufferInfos);
    }

    // Compute Ray Eval
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_computeRayEvalDescriptorSets[i] = std::make_shared<DescriptorSet>(m_device, m_computeRayEvalSetLayout,
            m_computeRayEvalPool);
        
        std::vector<VkDescriptorBufferInfo> bufferInfos = {
            m_creubo[i]->getInfo(),
            m_cressbo[i]->getInfo(),
#ifdef RAY_EVAL_DEBUG
            m_creDebugSsbo[i]->getInfo()
#endif
        };

        std::vector<uint32_t> bufferBinding = {
            0,
            1,
#ifdef RAY_EVAL_DEBUG
            2,
#endif
        };

        m_computeRayEvalDescriptorSets[i]->updateBuffers(bufferBinding, bufferInfos);

        std::vector<VkDescriptorImageInfo> imageInfos = {
            m_viewMatrixFramebuffer->getColorImageInfo(),
            m_viewMatrixFramebuffer->getDepthImageInfo(),
            getNovelImageInfo(),
            getTestPixelImageInfo()
        };

        std::vector<uint32_t> imageBinding = {
            3, 4, 5, 6
        };

        m_computeRayEvalDescriptorSets[i]->updateImages(imageBinding, imageInfos);

        m_pointCloudDescriptorsets[i] = std::make_shared<DescriptorSet>(m_device, m_pointCloudSetLayout,
            m_pointCloudPool);
        
        bufferInfos = {
            m_pointCloudUbo[i]->getInfo(),
            m_pointClouds[i]->getInfo()
        };

        bufferBinding = {
            0, 1
        };

        m_pointCloudDescriptorsets[i]->updateBuffers(bufferBinding, bufferInfos);
    }

    // Quad
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_quadDescriptorSets[i] = std::make_shared<DescriptorSet>(m_device, m_quadSetLayout,
            m_quadPool);
        
        std::vector<VkDescriptorBufferInfo> bufferInfos = {
            m_quadubo[i]->getInfo()
        };

        std::vector<uint32_t> bufferBinding = {
            0
        };

        m_quadDescriptorSets[i]->updateBuffers(bufferBinding, bufferInfos);

        std::vector<VkDescriptorImageInfo> imageInfos = {
            m_offscreenFramebuffer->getColorImageInfo()
        };

        std::vector<uint32_t> imageBinding = {
            1
        };

        m_quadDescriptorSets[i]->updateImages(imageBinding, imageInfos, 0);

        // secondary
        m_secondaryQuadDescriptorSets[i] = std::make_shared<DescriptorSet>(m_device, m_secondaryQuadSetLayout,
            m_secondaryQuadPool);
        
        bufferInfos = {
            m_secondaryQuadubo[i]->getInfo()
        };

        m_secondaryQuadDescriptorSets[i]->updateBuffers(bufferBinding, bufferInfos);

        imageInfos = {
            getNovelImageInfo()
        };

        m_secondaryQuadDescriptorSets[i]->updateImages(imageBinding, imageInfos, 0);

        QuadUniformBuffer quboData{};
        quboData.m_depthOnly = false;
        
        m_secondaryQuadubo[i]->copyMapped(&quboData, sizeof(QuadUniformBuffer));
    }
}

void Renderer::cullComputePass(const std::shared_ptr<Scene> &scene, const std::shared_ptr<ViewGrid>& viewGrid,
    bool novelViews)
{
    updateCullComputeDescriptorData(scene);

    std::vector<std::shared_ptr<View>> views = viewGrid->getViews();

    for (auto& view : views)
    {
        // view->getCamera()->reconstructMatrices();
        view->updateComputeDescriptorData(m_currentFrame, scene);

        if (!scene->viewResourcesExist(view))
        {
            scene->createViewResources(view, m_device, m_computeSceneSetLayout, m_computeScenePool);

            if (scene->getRenderDebugGeometryFlag() && !novelViews)
            {
                scene->setReinitializeDebugCameraGeometryFlag(true);
                scene->addDebugCameraGeometry(views);
            }
        }

        recordComputeCommandBuffer(m_computeCommandBuffers[m_currentFrame], scene, view);
    }
}

void Renderer::rayEvalComputePass(const std::shared_ptr<ViewGrid>& novelViewGrid, 
    const std::shared_ptr<ViewGrid>& viewGrid, const RayEvalParams& params)
{
    std::vector<std::shared_ptr<View>> novelViews = novelViewGrid->getViews();
    std::vector<std::shared_ptr<View>> views = viewGrid->getViews();

    updateRayEvalComputeDescriptorData(novelViews, views, params);

    glm::vec2 res = m_novelImage->getDims();//novelViews[0]->getResolution();

    m_device->createImageBarrier(m_computeCommandBuffers[m_currentFrame], 0, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_GENERAL, m_novelImage->getVkImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    VkDescriptorSet rayEvalSet = m_computeRayEvalDescriptorSets[m_currentFrame]->getDescriptorSet();

    vkCmdBindDescriptorSets(m_computeCommandBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, m_raysEvalPipeline->getPipelineLayout(),
        0, 1, &rayEvalSet, 0, nullptr);
    
    m_raysEvalPipeline->bind(m_computeCommandBuffers[m_currentFrame]);

    vkCmdDispatch(m_computeCommandBuffers[m_currentFrame], std::ceil(((double)res.x / INTERPOLATE_PIXELS_X) / 32.f), std::ceil(((double)res.y / INTERPOLATE_PIXELS_Y) / 32.f), 1);

#ifdef RAY_EVAL_DEBUG
    ViewEvalDebugCompute* evalData = (ViewEvalDebugCompute*)m_creDebugSsbo[m_currentFrame]->getMapped();

    int linearResId = (res.x * (res.y / 2.f)) + res.x / 2;
    std::cout << "Debug gpu info:" << std::endl;
    std::cout << "number of intersections: " << evalData[linearResId].numOfIntersections << std::endl;
    std::cout << "number of intervals: " << evalData[linearResId].numOfFoundIntervals << std::endl;
    std::cout << "res: " << evalData[linearResId].viewRes.x << " " << evalData[linearResId].viewRes.y << std::endl;
    std::cout << "p: " << evalData[linearResId].pointInWSpace.x << " " << 
        evalData[linearResId].pointInWSpace.y << " " << 
        evalData[linearResId].pointInWSpace.z << " " << 
        evalData[linearResId].pointInWSpace.w << std::endl;
#endif

}

void Renderer::pointCloudComputePass(std::shared_ptr<ViewGrid> &viewGrid)
{
    std::vector<std::shared_ptr<View>> views = viewGrid->getViews();
    
    VkDescriptorSet rayEvalSet = m_computeRayEvalDescriptorSets[m_currentFrame]->getDescriptorSet();
    VkDescriptorSet pointCloudSet = m_pointCloudDescriptorsets[m_currentFrame]->getDescriptorSet();

    vkCmdBindDescriptorSets(m_computeCommandBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, m_pointCloudCompPipeline->getPipelineLayout(),
        0, 1, &rayEvalSet, 0, nullptr);
    
    vkCmdBindDescriptorSets(m_computeCommandBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, m_pointCloudCompPipeline->getPipelineLayout(),
        0, 1, &pointCloudSet, 0, nullptr);

    m_pointCloudCompPipeline->bind(m_computeCommandBuffers[m_currentFrame]);

    vkCmdDispatch(m_computeCommandBuffers[m_currentFrame], std::ceil(((double)POINT_CLOUD_WIDTH) / 32.f), std::ceil(((double)POINT_CLOUD_HEIGHT) / 32.f), 1);
}

void Renderer::renderPass(const std::shared_ptr<Scene> &scene, const std::shared_ptr<ViewGrid>& viewGrid, 
        const std::shared_ptr<ViewGrid>& viewMatrixGrid, bool updateData)
{
    bool resizeViews = false;

    std::vector<std::shared_ptr<View>> views = viewGrid->getViews();
    std::vector<std::shared_ptr<View>> viewMatrix = viewMatrixGrid->getViews();

    if (updateData)
        updateDescriptorData(scene, views, viewMatrix);

    for (auto& view : views)
    {
        if (updateData)
            view->updateDescriptorData(m_currentFrame);

        glm::vec2 viewportStart = view->getViewportStart();
        glm::vec2 viewResolution = view->getResolution();

        setViewport(viewportStart, viewResolution);
        setScissor(viewportStart, viewResolution);

        recordCommandBuffer(m_commandBuffers[m_currentFrame], scene, view);
    }
}

void Renderer::quadRenderPass(glm::vec2 windowResolution, bool depthOnly, bool secondaryWindow)
{
    if (!secondaryWindow)
        updateQuadDescriptorData(depthOnly);

    setViewport(glm::vec2(0, 0), windowResolution);
    setScissor(glm::vec2(0, 0), windowResolution);

    m_quadPipeline->bind(m_commandBuffers[m_currentFrame]);

    VkDescriptorSet descriptorSet;
    if (!secondaryWindow)
        descriptorSet = m_quadDescriptorSets[m_currentFrame]->getDescriptorSet();
    else
        descriptorSet = m_secondaryQuadDescriptorSets[m_currentFrame]->getDescriptorSet();
    
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
    scissor.offset = { (int32_t)viewportStart.x, (int32_t)viewportStart.y };
    scissor.extent = { (uint32_t)viewportResolution.x, (uint32_t)viewportResolution.y };
    vkCmdSetScissor(m_commandBuffers[m_currentFrame], 0, 1, &scissor);
}

void Renderer::prepareFrame(const std::shared_ptr<Scene>& scene, std::shared_ptr<Window> window,
    bool secondarySwapchain)
{
    // Graphics part
    VkFence currentFence = m_swapChain->getFenceId(m_currentFrame);
    vkWaitForFences(m_device->getVkDevice(), 1, &currentFence, VK_TRUE, UINT64_MAX);
    //updateDescriptorData(scene, camera);

    VkSemaphore currentImageAvailableSemaphore = m_swapChain->getImageAvailableSemaphore(m_currentFrame);
    VkResult result = vkAcquireNextImageKHR(m_device->getVkDevice(), m_swapChain->getSwapChain(),
        UINT64_MAX, currentImageAvailableSemaphore, VK_NULL_HANDLE, &m_swapChainImageIndices[m_currentFrame]);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        VkExtent2D ext = window->getExtent();
        m_swapChain->recreate(ext);
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Error: failed to acquire swap chain image.");
    }

    if (secondarySwapchain)
    {
        result = vkAcquireNextImageKHR(m_device->getVkDevice(), m_secondarySwapchain->getSwapChain(),
            UINT64_MAX, m_secondarySwapchain->getImageAvailableSemaphore(m_currentFrame), VK_NULL_HANDLE, 
            &m_secondarySwapChainImageIndices[m_currentFrame]);
    }

    vkResetFences(m_device->getVkDevice(), 1, &currentFence);

    VkCommandBuffer currentCommandBuffer = m_commandBuffers[m_currentFrame];

    vkResetCommandBuffer(currentCommandBuffer, 0);
}

void Renderer::submitFrame(bool secondarySwapchain)
{
    VkCommandBuffer currentCommandBuffer = m_commandBuffers[m_currentFrame];

    VkSemaphore currentImageAvailableSemaphore = m_swapChain->getImageAvailableSemaphore(m_currentFrame);
    VkFence currentFence = m_swapChain->getFenceId(m_currentFrame);
    
    VkSemaphore currentComputeFinishedSemaphore = m_swapChain->getComputeFinishedSemaphore(m_currentFrame);
    
    std::vector<VkSemaphore> waitSemaphores = {
        currentComputeFinishedSemaphore,
        currentImageAvailableSemaphore
    };

    std::vector<VkPipelineStageFlags> waitStages = {
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    if (secondarySwapchain)
    {
        waitSemaphores.push_back(m_secondarySwapchain->getImageAvailableSemaphore(m_currentFrame));
        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
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

void Renderer::submitGraphics()
{
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];
    VkCommandBuffer currentCommandBuffer = commandBuffer;

    VkFence currentFence = m_swapChain->getFenceId(m_currentFrame);
    VkSemaphore currentComputeFinishedSemaphore = m_swapChain->getComputeFinishedSemaphore(m_currentFrame);

    VkSemaphore waitSemaphores[] = {
        currentComputeFinishedSemaphore
    };
    VkPipelineStageFlags waitStages[] = { 
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
    };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentCommandBuffer;

    VkResult res = vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

void Renderer::submitCompute()
{
    VkFence currentComputeFence = m_swapChain->getComputeFenceId(m_currentFrame);

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

void Renderer::presentFrame(std::shared_ptr<Window> window, bool secondarySwapchain)
{
    VkSemaphore currentRenderFinishedSemaphore = m_swapChain->getRenderFinishedSemaphore(m_currentFrame);
    VkSemaphore signalSemaphores[] = {currentRenderFinishedSemaphore};

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    std::vector<VkSwapchainKHR> swapChains;
    swapChains.push_back(m_swapChain->getSwapChain());

    std::vector<uint32_t> swapchainIndices = {
        m_swapChainImageIndices[m_currentFrame]
    };

    if (secondarySwapchain)
    {
        swapChains.push_back(m_secondarySwapchain->getSwapChain());
        swapchainIndices.push_back(m_secondarySwapChainImageIndices[m_currentFrame]);
    }

    presentInfo.swapchainCount = swapChains.size();
    presentInfo.pSwapchains = swapChains.data();
    presentInfo.pImageIndices = swapchainIndices.data();
    presentInfo.pResults = nullptr;

    VkResult result = vkQueuePresentKHR(m_device->getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->resized())
    {
        VkExtent2D ext = window->getExtent();
        m_swapChain->recreate(ext);

        m_window->setResized(false);
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image");
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::changeQuadRenderPassSource(VkDescriptorImageInfo imageInfo)
{
    std::vector<VkDescriptorImageInfo> imageInfos{
        imageInfo
    };

    std::vector<uint32_t> imageBinding{
        1
    };

    m_quadDescriptorSets[m_currentFrame]->updateImages(imageBinding, imageInfos, 0);
}

void Renderer::copyOffscreenFrameBufferToSupp()
{
    m_device->createImageBarrier(m_commandBuffers[m_currentFrame], VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_viewMatrixFramebuffer->getColorImage()->getVkImage(),
        VK_IMAGE_ASPECT_COLOR_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    
    m_device->createImageBarrier(m_commandBuffers[m_currentFrame], 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        m_testPixelImage->getVkImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkImageCopy imageCopyRegion{};
    imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopyRegion.srcSubresource.layerCount = 1;
    imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopyRegion.dstSubresource.layerCount = 1;
    imageCopyRegion.extent.width = FRAMEBUFFER_WIDTH;
    imageCopyRegion.extent.height = FRAMEBUFFER_HEIGHT;
    imageCopyRegion.extent.depth = 1;

    // Issue the copy command
    vkCmdCopyImage(
        m_commandBuffers[m_currentFrame],
        m_viewMatrixFramebuffer->getColorImage()->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        m_testPixelImage->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageCopyRegion);

    m_device->createImageBarrier(m_commandBuffers[m_currentFrame], VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_viewMatrixFramebuffer->getColorImage()->getVkImage(), VK_IMAGE_ASPECT_COLOR_BIT, 
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_device->createImageBarrier(m_commandBuffers[m_currentFrame], VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL, m_testPixelImage->getVkImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);    
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

std::shared_ptr<RenderPass> Renderer::getOffscreenRenderPass() const
{
    return m_offscreenRenderPass;
}

std::shared_ptr<RenderPass> Renderer::getQuadRenderPass() const
{
return m_quadRenderPass;
}

std::shared_ptr<Framebuffer> Renderer::getOffscreenFramebuffer() const
{
    return m_offscreenFramebuffer;
}

std::shared_ptr<Framebuffer> Renderer::getQuadFramebuffer() const
{
    return m_swapChain->getFramebuffer(m_swapChainImageIndices[m_currentFrame]);
}

std::shared_ptr<Framebuffer> Renderer::getSecondaryQuadFramebuffer() const
{
    return m_secondarySwapchain->getFramebuffer(m_secondarySwapChainImageIndices[m_currentFrame]);
}

std::shared_ptr<Framebuffer> Renderer::getViewMatrixFramebuffer() const
{
    return m_viewMatrixFramebuffer;
}

VkDescriptorImageInfo Renderer::getNovelImageInfo() const
{
    return VkDescriptorImageInfo{
        m_novelImageSampler->getVkSampler(),
        m_novelImageView,
        m_novelImage->getVkImageLayout()
    };
}

VkDescriptorImageInfo Renderer::getTestPixelImageInfo() const
{
    return VkDescriptorImageInfo{
        m_testPixelSampler->getVkSampler(),
        m_testPixelImageView,
        m_testPixelImage->getVkImageLayout()
    };
}

SamplingType Renderer::getNovelViewSamplingType() const
{
    return m_novelViewSamplingType;
}

void Renderer::setSceneChanged(int sceneChanged)
{
    m_sceneFramesUpdated = sceneChanged;
}

void Renderer::setLightChanged(int lightChanged)
{
    m_lightsFramesUpdated = lightChanged;
}

void Renderer::setNovelViewSamplingType(SamplingType samplingType)
{
    m_novelViewSamplingType = samplingType;
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

void Renderer::addSecondaryWindow(std::shared_ptr<Window> window)
{
    m_secondarySwapChainImageIndices.resize(MAX_FRAMES_IN_FLIGHT);
    m_secondarySwapchain = std::make_shared<SwapChain>(m_device, window->getExtent(), window->getSurface());
    m_secondarySwapchain->initializeFramebuffers(m_quadRenderPass);
}

void Renderer::beginComputePass()
{
    // Compute part
    VkFence currentComputeFence = m_swapChain->getComputeFenceId(m_currentFrame);
    vkWaitForFences(m_device->getVkDevice(), 1, &currentComputeFence, VK_TRUE, UINT64_MAX);

    vkResetFences(m_device->getVkDevice(), 1, &currentComputeFence);

    vkResetCommandBuffer(m_computeCommandBuffers[m_currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_computeCommandBuffers[m_currentFrame], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin compute command buffer");
}

void Renderer::endComputePass()
{
    if (vkEndCommandBuffer(m_computeCommandBuffers[m_currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("failed to end compute command buffer");
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

void Renderer::beginRenderPass(std::shared_ptr<RenderPass> renderPass, std::shared_ptr<Framebuffer> framebuffer)
{
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    renderPassInfo.renderPass = renderPass->getRenderPass();

    VkExtent2D res = framebuffer->getResolution();

    renderPassInfo.framebuffer = framebuffer->getFramebuffer();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = res;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.f, 0.f, 0.f, 1.f} };
    clearValues[1].depthStencil = { 1.f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::setNovelViewBarrier()
{
    m_device->createImageBarrier(m_commandBuffers[m_currentFrame], 0, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_GENERAL, m_novelImage->getVkImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
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

        m_quadubo[i] = std::make_unique<Buffer>(m_device, sizeof(QuadUniformBuffer),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_quadubo[i]->map();

        m_secondaryQuadubo[i] = std::make_unique<Buffer>(m_device, sizeof(QuadUniformBuffer),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_secondaryQuadubo[i]->map();
    }

    VkDescriptorSetLayoutBinding vssboLayoutBinding = createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1, VK_SHADER_STAGE_VERTEX_BIT);
    VkDescriptorSetLayoutBinding fuboLayoutBinding = createDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkDescriptorSetLayoutBinding fssboLayoutBinding = createDescriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1, VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {
        vssboLayoutBinding,
        fuboLayoutBinding,
        fssboLayoutBinding,
    };

    m_descriptorSetLayout = std::make_shared<DescriptorSetLayout>(m_device, layoutBindings);

    VkDescriptorPoolSize vssboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_SBOS));
    VkDescriptorPoolSize fuboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    VkDescriptorPoolSize fssboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_SBOS));

    std::vector<VkDescriptorPoolSize> poolSizes = {
        vssboPoolSize,
        fuboPoolSize,
        fssboPoolSize
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

    // Compute buffers
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_cssbos[i] = std::make_unique<Buffer>(m_device, sizeof(MeshShaderDataCompute) * MAX_SBOS,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_cssbos[i]->map();

        m_creubo[i] = std::make_unique<Buffer>(m_device, sizeof(RayEvalUniformBuffer), 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_creubo[i]->map();

        m_cressbo[i] = std::make_unique<Buffer>(m_device, sizeof(ViewEvalDataCompute) * MAX_VIEWS, 
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_cressbo[i]->map();

        m_pointCloudUbo[i] = std::make_unique<Buffer>(m_device, sizeof(PointCloudUniformBuffer), 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_pointCloudUbo[i]->map();

        m_pointClouds[i] = std::make_unique<Buffer>(m_device, MAX_VIEWS * MAX_OFFSCREEN_RESOLUTION_LINEAR * (sizeof(Point)),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_pointClouds[i]->map();

#ifdef RAY_EVAL_DEBUG
        m_creDebugSsbo[i] = std::make_unique<Buffer>(m_device, sizeof(ViewEvalDebugCompute) * MAX_RESOLUTION_LINEAR, 
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_creDebugSsbo[i]->map();
#endif

        // m_creHitsssbo[i] = std::make_unique<Buffer>(m_device, sizeof(FrustumHit) * MAX_RESOLUTION_LINEAR * MAX_VIEWS * 2, 
        //     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        // m_creHitsssbo[i]->map();
        // 
        // m_creHitsCntssbo[i] = std::make_unique<Buffer>(m_device, sizeof(int) * MAX_RESOLUTION_LINEAR, 
        //     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        // m_creHitsCntssbo[i]->map();
    }

    // Compute general
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

    // Compute raygen
    VkDescriptorSetLayoutBinding uboRayGenLayoutBinding = createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1, VK_SHADER_STAGE_COMPUTE_BIT);
    VkDescriptorSetLayoutBinding ssboRayGenLayoutBinding = createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1, VK_SHADER_STAGE_COMPUTE_BIT);
#ifdef RAY_EVAL_DEBUG
    VkDescriptorSetLayoutBinding ssboDebugRayGenLayoutBinding = createDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1, VK_SHADER_STAGE_COMPUTE_BIT);
#endif
    VkDescriptorSetLayoutBinding viewsFramebRayGenLayoutBinding = createDescriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        1, VK_SHADER_STAGE_COMPUTE_BIT);
    VkDescriptorSetLayoutBinding viewsFramebDepthRayGenLayoutBinding = createDescriptorSetLayoutBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        1, VK_SHADER_STAGE_COMPUTE_BIT);
    VkDescriptorSetLayoutBinding novelFramebRayGenLayoutBinding = createDescriptorSetLayoutBinding(5, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        1, VK_SHADER_STAGE_COMPUTE_BIT);
    VkDescriptorSetLayoutBinding testPixelRayGenLayoutBinding = createDescriptorSetLayoutBinding(6, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        1, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> computeRayGenLayoutBindings = {
        uboRayGenLayoutBinding,
        ssboRayGenLayoutBinding,
#ifdef RAY_EVAL_DEBUG
        ssboDebugRayGenLayoutBinding,
#endif
        viewsFramebRayGenLayoutBinding,
        viewsFramebDepthRayGenLayoutBinding,
        novelFramebRayGenLayoutBinding,
        testPixelRayGenLayoutBinding
    };

    m_computeRayEvalSetLayout = std::make_shared<DescriptorSetLayout>(m_device, computeRayGenLayoutBindings);
    
    VkDescriptorPoolSize uboRayGenPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    VkDescriptorPoolSize ssboRayGenPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_VIEWS));
#ifdef RAY_EVAL_DEBUG
    VkDescriptorPoolSize ssboDebugRayGenPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(MAX_RESOLUTION_LINEAR));
#endif
    VkDescriptorPoolSize viewsFramebRayGenPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    VkDescriptorPoolSize viewsFramebDepthRayGenPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    VkDescriptorPoolSize novelFramebRayGenPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    VkDescriptorPoolSize testPixelbRayGenPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    

    std::vector<VkDescriptorPoolSize> computeRayGenSizes = {
        uboRayGenPoolSize,
        ssboRayGenPoolSize,
#ifdef RAY_EVAL_DEBUG
        ssboDebugRayGenPoolSize,
#endif
        viewsFramebRayGenPoolSize,
        viewsFramebDepthRayGenPoolSize,
        novelFramebRayGenPoolSize,
        testPixelbRayGenPoolSize
    };

    m_computeRayEvalPool = std::make_shared<DescriptorPool>(m_device, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), 0,
        computeRayGenSizes);

    // quad
    VkDescriptorSetLayoutBinding quboLayoutBinding = createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkDescriptorSetLayoutBinding finalImageLayoutBinding = createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        1, VK_SHADER_STAGE_FRAGMENT_BIT);
    
    std::vector<VkDescriptorSetLayoutBinding> quadBindings = {
        quboLayoutBinding,
        finalImageLayoutBinding
    };

    VkDescriptorBindingFlags finalImageFlags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

    std::vector<VkDescriptorBindingFlags> quadBindingFlags = {
        0,
        finalImageFlags
    };

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT quadLayoutNext{};
    quadLayoutNext.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    quadLayoutNext.bindingCount = quadBindingFlags.size();
    quadLayoutNext.pBindingFlags = quadBindingFlags.data();
    quadLayoutNext.pNext = nullptr;

    m_quadSetLayout = std::make_shared<DescriptorSetLayout>(m_device, quadBindings,
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, &quadLayoutNext);
    
    VkDescriptorPoolSize quboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    VkDescriptorPoolSize finalImagePoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

    std::vector<VkDescriptorPoolSize> quadPoolSizes = {
        quboPoolSize,
        finalImagePoolSize
    };

    m_quadPool = std::make_shared<DescriptorPool>(m_device, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), 
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, quadPoolSizes);

    // second window
    m_secondaryQuadSetLayout = std::make_shared<DescriptorSetLayout>(m_device, quadBindings,
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, &quadLayoutNext);
    m_secondaryQuadPool = std::make_shared<DescriptorPool>(m_device, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), 
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, quadPoolSizes);

    // point clouds
    VkDescriptorSetLayoutBinding pointCloudUboLayoutBinding = createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1, VK_SHADER_STAGE_COMPUTE_BIT);
    VkDescriptorSetLayoutBinding pointCloudLayoutBinding = createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT);

    std::vector<VkDescriptorSetLayoutBinding> pointCloudLayoutBindings = {
        pointCloudUboLayoutBinding,
        pointCloudLayoutBinding
    };

    m_pointCloudSetLayout = std::make_shared<DescriptorSetLayout>(m_device, pointCloudLayoutBindings);

    VkDescriptorPoolSize pointCloudUboPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
    VkDescriptorPoolSize pointCloudPoolSize = createPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * MAX_VIEWS);

    std::vector<VkDescriptorPoolSize> pointCloudGenSizes = {
        pointCloudUboPoolSize,
        pointCloudPoolSize
    };

    m_pointCloudPool = std::make_shared<DescriptorPool>(m_device, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), 0,
        pointCloudGenSizes);

}

void Renderer::createRenderResources()
{
    VkFormat depthFormat = m_device->getDepthFormat();

    m_swapChain = std::make_shared<SwapChain>(m_device, m_window->getExtent(), m_window->getSurface());
    m_quadRenderPass = std::make_shared<RenderPass>(m_device, m_swapChain->getImageFormat(), depthFormat);
    m_swapChain->initializeFramebuffers(m_quadRenderPass);

    m_offscreenRenderPass = std::make_shared<RenderPass>(m_device, VK_FORMAT_R8G8B8A8_UNORM,
        depthFormat, true);

    m_offscreenFramebuffer = std::make_shared<Framebuffer>(m_device, m_offscreenRenderPass,
        VkExtent2D{(uint32_t)FRAMEBUFFER_WIDTH, (uint32_t)FRAMEBUFFER_HEIGHT});

    m_viewMatrixFramebuffer = std::make_shared<Framebuffer>(m_device, m_offscreenRenderPass,
        VkExtent2D{(uint32_t)FRAMEBUFFER_WIDTH, (uint32_t)FRAMEBUFFER_HEIGHT});

    m_novelImage = std::make_shared<Image>(m_device, glm::vec2(NOVEL_VIEW_WIDTH, NOVEL_VIEW_HEIGHT), VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_novelImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
    m_novelImageView = m_novelImage->createImageView(VK_IMAGE_ASPECT_COLOR_BIT);
    m_novelImageSampler = std::make_shared<Sampler>(m_device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        VK_SAMPLER_MIPMAP_MODE_LINEAR);

    m_testPixelImage = std::make_shared<Image>(m_device, glm::vec2(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT), VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_testPixelImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
    m_testPixelImageView = m_testPixelImage->createImageView(VK_IMAGE_ASPECT_COLOR_BIT);
    m_testPixelSampler = std::make_shared<Sampler>(m_device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        VK_SAMPLER_MIPMAP_MODE_LINEAR);

    // m_offscreenSuppImage = std::make_shared<Image>(m_device, glm::vec2(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT), VK_FORMAT_R8G8B8A8_UNORM,
    //     VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    // m_offscreenSuppImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
    // m_offscreenSuppView = m_novelImage->createImageView(VK_IMAGE_ASPECT_COLOR_BIT);
    // m_offscreenSuppSampler = std::make_shared<Sampler>(m_device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    //     VK_SAMPLER_MIPMAP_MODE_LINEAR);
    
}

void Renderer::createPipeline(const RendererInitParams& params)
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
        m_quadSetLayout->getLayout(),
        m_secondaryQuadSetLayout->getLayout()
    };

    std::vector<VkDescriptorSetLayout> computeRaysEvalSetLayout = {
        m_computeRayEvalSetLayout->getLayout()
    };

    std::vector<VkDescriptorSetLayout> pointCloudComputeSetLayout = {
        m_computeRayEvalSetLayout->getLayout(),
        m_pointCloudSetLayout->getLayout()
    };

    m_offscreenPipeline = std::make_shared<GraphicsPipeline>(m_device, m_offscreenRenderPass->getRenderPass(), params.vertexShaderFile,
        params.fragmentShaderFile, offscreenGraphicsSetLayouts);

    m_quadPipeline = std::make_shared<GraphicsPipeline>(m_device, m_quadRenderPass->getRenderPass(), params.quadVertexShaderFile,
        params.quadFragmentShaderFile, quadSetLayout, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false, false);
    
    m_cullPipeline = std::make_shared<ComputePipeline>(m_device, params.computeShaderFile, computeSetLayouts);

    m_raysEvalPipeline = std::make_shared<ComputePipeline>(m_device, params.computeRaysEvalShaderFile, computeRaysEvalSetLayout);

    // Point clouds
    m_pointCloudCompPipeline = std::make_shared<ComputePipeline>(m_device, params.computePointCloudShaderFile, pointCloudComputeSetLayout);
    // m_pointCloudGraphPipeline = std::make_shared<GraphicsPipeline>

    // testing
    // m_intersectsPipeline = std::make_shared<ComputePipeline>(m_device, "intersect.spv", computeRaysEvalSetLayout);
    // m_intervalsPipeline = std::make_shared<ComputePipeline>(m_device, "intervalsInterpolate.spv", computeRaysEvalSetLayout);
}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<Scene>& scene,
    const std::shared_ptr<View>& view)
{
    m_offscreenPipeline->bind(commandBuffer);

    VkDescriptorSet descriptorSet = m_generalDescriptorSets[m_currentFrame]->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_offscreenPipeline->getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

    VkDescriptorSet textureSet = m_materialDescriptorSets[m_currentFrame]->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_offscreenPipeline->getPipelineLayout(), 1, 1, &textureSet, 0, nullptr);

    VkDescriptorSet viewSet = view->getViewDescriptorSet(m_currentFrame)->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_offscreenPipeline->getPipelineLayout(), 2, 1, &viewSet, 0, nullptr);

    scene->draw(view, commandBuffer, m_currentFrame);
}

void Renderer::recordComputeCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<Scene>& scene,
    const std::shared_ptr<View>& view)
{
    VkDescriptorSet computeSet = m_computeDescriptorSets[m_currentFrame]->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_cullPipeline->getPipelineLayout(), 0, 1, &computeSet, 0, nullptr);

    VkDescriptorSet viewSet = view->getViewDescriptorSet(m_currentFrame)->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_cullPipeline->getPipelineLayout(), 2, 1, &viewSet, 0, nullptr);

    m_cullPipeline->bind(commandBuffer);

    scene->dispatch(view, commandBuffer, m_cullPipeline->getPipelineLayout(), m_currentFrame);
}

void Renderer::updateDescriptorData(const std::shared_ptr<Scene>& scene, const std::vector<std::shared_ptr<View>>& views,
    const std::vector<std::shared_ptr<View>>& viewMatrix)
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

    if (scene->sceneChanged())
    {
        // std::cout << "update" << std::endl;

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
            for (auto& view : viewMatrix)
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

void Renderer::updateCullComputeDescriptorData(const std::shared_ptr<Scene> &scene)
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

void Renderer::updateRayEvalComputeDescriptorData(const std::vector<std::shared_ptr<View>>& novelViews,
        const std::vector<std::shared_ptr<View>>& views, const RayEvalParams& params)
{
    std::shared_ptr<Camera> mainCamera = novelViews[0]->getCamera();
    glm::vec2 res = m_novelImage->getDims();
    VkExtent2D offscreenFbRes = m_offscreenFramebuffer->getResolution();

    RayEvalUniformBuffer creuData{};
    creuData.invProj = mainCamera->getProjectionInverse();
    creuData.invView = mainCamera->getViewInverse();
    creuData.res = res;
    creuData.viewsTotalRes = glm::vec2(offscreenFbRes.width, offscreenFbRes.height);
    creuData.viewCnt = views.size();
    creuData.samplingType = 1 << static_cast<int>(m_novelViewSamplingType);
    creuData.testPixel = params.testPixel;
    creuData.testedPixel = params.testedPixel;
    creuData.numOfRaySamples = params.numOfRaySamples;
    creuData.automaticSampleCount = params.automaticSampleCount;
    creuData.thresholdDepth = params.thresholdDepth;
    creuData.maxSampleDistance = params.maxSampleDistance;

    m_creubo[m_currentFrame]->copyMapped(&creuData, sizeof(RayEvalUniformBuffer));

    std::vector<ViewEvalDataCompute> cressbo(views.size());

    for (int i = 0; i < views.size(); i++)
    {
        std::vector<glm::vec4> planes = views[i]->getCamera()->getFrustumPlanes();
        memcpy(cressbo[i].frustumPlanes, planes.data(), sizeof(glm::vec4) * 6);
        cressbo[i].view = views[i]->getCamera()->getView();
        cressbo[i].proj = views[i]->getCamera()->getProjection();
        cressbo[i].invView = views[i]->getCamera()->getViewInverse();
        cressbo[i].invProj = views[i]->getCamera()->getProjectionInverse();

        glm::vec2 res = views[i]->getResolution();
        glm::vec2 offset = views[i]->getViewportStart();
        cressbo[i].resOffset.x = res.x;
        cressbo[i].resOffset.y = res.y;
        cressbo[i].resOffset.z = offset.x;
        cressbo[i].resOffset.w = offset.y;
        cressbo[i].nearFar = views[i]->getNearFar();
        glm::vec3 viewDir = views[i]->getCamera()->getTransfViewDir();
        cressbo[i].viewDir = glm::vec4(viewDir.x, viewDir.y, viewDir.z, 0.f);
    }

    m_cressbo[m_currentFrame]->copyMapped(cressbo.data(), sizeof(ViewEvalDataCompute) * cressbo.size());

}

void Renderer::updateQuadDescriptorData(bool depthOnly){
    QuadUniformBuffer quboData{};
    quboData.m_depthOnly = depthOnly;
    
    m_quadubo[m_currentFrame]->copyMapped(&quboData, sizeof(QuadUniformBuffer));
};

}