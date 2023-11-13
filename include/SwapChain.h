#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"

namespace vke
{

class Image;

class SwapChain
{
public:
    SwapChain(std::shared_ptr<Device> device, VkExtent2D windowExtent);
    ~SwapChain();

    VkRenderPass getRenderPass();
    VkFence getFenceId(int id);
    VkFence getComputeFenceId(int id);
    VkSwapchainKHR getSwapChain();
    VkSemaphore getImageAvailableSemaphore(int id);
    VkSemaphore getRenderFinishedSemaphore(int id);
    VkSemaphore getComputeFinishedSemaphore(int id);
    VkFramebuffer getFramebuffer(int id);
    VkExtent2D getExtent();
    uint32_t getImageCount() const;

    // Sync members
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    // Compute sync members
    std::vector<VkSemaphore> m_computeFinishedSemaphores;
    std::vector<VkFence> m_computeInFlightFences;
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

    uint32_t m_imageCount;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    // Depth buffer
    std::shared_ptr<Image> m_depthImage;
    VkImageView m_depthImageView;
};

}