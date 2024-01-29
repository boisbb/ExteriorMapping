#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"

namespace vke
{

class Image;
class Sampler;
class RenderPass;
class Framebuffer;

class SwapChain
{
public:
    SwapChain(std::shared_ptr<Device> device, VkExtent2D windowExtent);
    ~SwapChain();

    void initializeFramebuffers(std::shared_ptr<RenderPass> renderPass);

    // VkRenderPass getRenderPass() const;
    // VkRenderPass getOffscreenRenderPass() const;
    VkFence getFenceId(int id);
    VkFence getComputeFenceId(int id);
    VkSwapchainKHR getSwapChain() const;
    VkSemaphore getImageAvailableSemaphore(int id);
    VkSemaphore getRenderFinishedSemaphore(int id);
    VkSemaphore getComputeFinishedSemaphore(int id);
    std::shared_ptr<Framebuffer> getFramebuffer(int id);
    // VkFramebuffer getOffscreenFramebuffer() const;
    VkExtent2D getExtent();
    uint32_t getImageCount() const;
    // VkDescriptorImageInfo getOffscreenImageInfo() const;
    VkFormat getImageFormat() const;

    void recreate(VkExtent2D windowExtent);

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
    void createSyncObjects();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    bool hasStencilComponent(VkFormat format);

    void cleanup();

    std::shared_ptr<Device> m_device;
    
    VkFormat m_depthFormat;

    VkExtent2D m_windowExtent;

    VkSwapchainKHR m_swapChain;
    std::shared_ptr<RenderPass> m_renderPass;

    // std::shared_ptr<RenderPass> m_offscreenRenderPass;
    // std::shared_ptr<Framebuffer> m_offscreenFramebuffer;

    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    uint32_t m_imageCount;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<std::shared_ptr<Framebuffer>> m_swapChainFramebuffers;

    // Depth buffer
    std::shared_ptr<Image> m_depthImage;
    VkImageView m_depthImageView;
};

}