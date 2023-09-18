#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"

namespace vke
{

class SwapChain
{
public:
    SwapChain(std::shared_ptr<Device> device, VkExtent2D windowExtent);
    ~SwapChain();

    VkRenderPass getRenderPass();
    VkFence getFenceId(int id);
    VkSwapchainKHR getSwapChain();
    VkSemaphore getImageAvailableSemaphore(int id);
    VkSemaphore getRenderFinishedSemaphore(int id);
    VkFramebuffer getFramebuffer(int id);
    VkExtent2D getExtent();
private:
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
        VkFormatFeatureFlags features);
    VkFormat findDepthFormat();

    bool hasStencilComponent(VkFormat format);

    std::shared_ptr<Device> m_device;

    VkExtent2D m_windowExtent;

    VkSwapchainKHR m_swapChain;
    VkRenderPass m_renderPass;

    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    // Depth buffer
    VkImage m_depthImage;
    VkDeviceMemory m_depthImageMemory;
    VkImageView m_depthImageView;

    // Sync members
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
};

}