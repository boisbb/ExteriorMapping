/**
 * @file SwapChain.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

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
    /**
     * @brief Construct a new Swap Chain object
     * 
     * @param device Device.
     * @param windowExtent Resolution of the swap chain object. 
     * @param surface Surface to render to.
     */
    SwapChain(std::shared_ptr<Device> device, VkExtent2D windowExtent,
        VkSurfaceKHR surface);
    ~SwapChain();

    /**
     * @brief Initializes frame buffers.
     * 
     * @param renderPass 
     */
    void initializeFramebuffers(std::shared_ptr<RenderPass> renderPass);

    // Getters
    VkFence getFenceId(int id);
    VkFence getComputeFenceId(int id);
    VkSwapchainKHR getSwapChain() const;
    VkSemaphore getImageAvailableSemaphore(int id);
    VkSemaphore getRenderFinishedSemaphore(int id);
    VkSemaphore getComputeFinishedSemaphore(int id);
    std::shared_ptr<Framebuffer> getFramebuffer(int id);
    VkExtent2D getExtent();
    uint32_t getImageCount() const;
    VkFormat getImageFormat() const;

    /**
     * @brief Recreates the swapchain.
     * 
     * @param windowExtent 
     */
    void recreate(VkExtent2D windowExtent);

    // Sync members
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    // Compute sync members
    std::vector<VkSemaphore> m_computeFinishedSemaphores;
    std::vector<VkFence> m_computeInFlightFences;
private:
    // Create methods.
    void createSwapChain(VkSurfaceKHR surface);
    void createImageViews();
    void createSyncObjects();

    /**
     * @brief Chooses appropriatte surface format.
     * 
     * @param availableFormats 
     * @return VkSurfaceFormatKHR 
     */
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    /**
     * @brief Chooses present mode.
     * 
     * @param availablePresentModes 
     * @return VkPresentModeKHR 
     */
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    /**
     * @brief Chooses swap extent.
     * 
     * @param capabilities 
     * @return VkExtent2D 
     */
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    bool hasStencilComponent(VkFormat format);

    void cleanup();

    std::shared_ptr<Device> m_device;
    
    VkFormat m_depthFormat;
    VkExtent2D m_windowExtent;
    VkSwapchainKHR m_swapChain;

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