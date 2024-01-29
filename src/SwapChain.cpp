#include "SwapChain.h"
#include "Device.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "Image.h"
#include "Sampler.h"
#include "utils/Structs.h"
#include "utils/Constants.h"

#include <limits>
#include <algorithm>

namespace vke
{

SwapChain::SwapChain(std::shared_ptr<Device> device, VkExtent2D windowExtent)
    : m_device(device), m_windowExtent(windowExtent)
{
    m_depthFormat = m_device->getDepthFormat();

    createSwapChain();
    createImageViews();
    createSyncObjects();
}

SwapChain::~SwapChain()
{
}

void SwapChain::initializeFramebuffers(std::shared_ptr<RenderPass> renderPass)
{
    for (int i = 0; i < m_imageCount; i++)
    {
        m_swapChainFramebuffers.push_back(std::make_shared<Framebuffer>(m_device, renderPass, m_swapChainExtent,
            m_swapChainImageViews[i]));
    }
}

// VkRenderPass SwapChain::getRenderPass() const
// {
//     return m_renderPass->getRenderPass();
// }

// VkRenderPass SwapChain::getOffscreenRenderPass() const
// {
//     return m_offscreenRenderPass->getRenderPass();
// }

void SwapChain::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = m_device->getSwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_device->getSurface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = m_device->getQueueFamilies();

    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(m_device->getVkDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swapchain!");
    }

    vkGetSwapchainImagesKHR(m_device->getVkDevice(), m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device->getVkDevice(), m_swapChain, &imageCount, m_swapChainImages.data());

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;

    m_imageCount = imageCount;
}

void SwapChain::createImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++)
    {
        m_swapChainImageViews[i] = m_device->createImageView(m_swapChainImages[i], m_swapChainImageFormat,
            VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void SwapChain::createSyncObjects()
{
    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    m_computeFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_computeInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(m_device->getVkDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS
        ||  vkCreateSemaphore(m_device->getVkDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS
        ||  vkCreateFence(m_device->getVkDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores!");
        }

        if (vkCreateSemaphore(m_device->getVkDevice(), &semaphoreInfo, nullptr, &m_computeFinishedSemaphores[i]) != VK_SUCCESS
            || vkCreateFence(m_device->getVkDevice(), &fenceInfo, nullptr, &m_computeInFlightFences[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create compute semaphores");
    }
}

VkFence SwapChain::getFenceId(int id)
{
    return m_inFlightFences[id];
}

VkFence SwapChain::getComputeFenceId(int id)
{
    return m_computeInFlightFences[id];
}

VkSwapchainKHR SwapChain::getSwapChain() const
{
    return m_swapChain;
}

VkSemaphore SwapChain::getImageAvailableSemaphore(int id)
{
    return m_imageAvailableSemaphores[id];
}

VkSemaphore SwapChain::getRenderFinishedSemaphore(int id)
{
    return m_renderFinishedSemaphores[id];
}

VkSemaphore SwapChain::getComputeFinishedSemaphore(int id)
{
    return m_computeFinishedSemaphores[id];
}

std::shared_ptr<Framebuffer> SwapChain::getFramebuffer(int id)
{
    return m_swapChainFramebuffers[id];
}

// VkFramebuffer SwapChain::getOffscreenFramebuffer() const
// {
//     return m_offscreenFramebuffer->getFramebuffer();
// }

VkExtent2D SwapChain::getExtent()
{
    return m_swapChainExtent;
}

uint32_t SwapChain::getImageCount() const
{
    return m_imageCount;
}

// VkDescriptorImageInfo SwapChain::getOffscreenImageInfo() const
// {
//     VkDescriptorImageInfo info{};
//     info.sampler = m_offscreenFramebuffer->getSampler()->getVkSampler();
//     info.imageView = m_offscreenFramebuffer->getColorImageView();
//     info.imageLayout = m_offscreenFramebuffer->getColorImage()->getVkImageLayout();
// 
//     return info;
// }

VkFormat SwapChain::getImageFormat() const
{
    return m_swapChainImageFormat;
}

void SwapChain::recreate(VkExtent2D windowExtent)
{
    vkDeviceWaitIdle(m_device->getVkDevice());

    m_windowExtent = windowExtent;

    cleanup();

    createSwapChain();
    // createImageViews();
    // createDepthResources();
    // createFramebuffers();

}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB 
        && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }
    
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else 
    {
        VkExtent2D actualExtent;

        actualExtent.width = std::clamp(m_windowExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(m_windowExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

bool SwapChain::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void SwapChain::cleanup()
{
    // for (int i = 0; i < m_swapChainFramebuffers.size(); i++)
    // {
    //     vkDestroyFramebuffer(m_device->getVkDevice(), m_swapChainFramebuffers[i], nullptr);
    // }

    // for (int i = 0; i < m_swapChainImageViews.size(); i++)
    // {
    //     vkDestroyImageView(m_device->getVkDevice(), m_swapChainImageViews[i], nullptr);
    // }

    vkDestroySwapchainKHR(m_device->getVkDevice(), m_swapChain, nullptr);
}

}