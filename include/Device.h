/**
 * @file Device.h
 * @author Boris Burkalo (xburka00)
 * @brief Abstracts Vulkan device actions.
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

// std
#include <vector>
#include <iostream>
#include <optional>
#include <fstream>
#include <array>
#include <memory>

// Vulkan
#include <vulkan/vulkan.h>

// GLFW
#ifdef linux
#define VK_USE_PLATFORM_XCB_KHR
#elif _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#ifdef linux
#define GLFW_EXPOSE_NATIVE_X11
#elif _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

// vke
#include "Window.h"
#include "utils/Structs.h"

namespace vke
{

class Buffer;
class Image;

class Device
{
public:
    /**
     * @brief Construct a new Device object.
     * 
     * @param window Window which will be used for rendering.
     */
    Device(std::shared_ptr<Window> window);
    ~Device();

    void destroyVkResources();

    // Getters
    QueueFamilyIndices getQueueFamilies();
    QueueFamilyIndices getQueueFamilyIndices();
    SwapChainSupportDetails getSwapChainSupport();
    VkSurfaceKHR getSurface() const;
    VkDevice getVkDevice() const;
    VkInstance getInstance() const;
    VkCommandPool getCommandPool() const;
    VkQueue getGraphicsQueue() const;
    VkQueue getPresentQueue() const;
    VkQueue getComputeQueue() const;
    VkPhysicalDevice getPhysicalDevice() const;
    VkPhysicalDeviceFeatures getFeatures() const;
    VkFormat getDepthFormat() const;

    /**
     * @brief Finds physical device memory type.
     * 
     * @param typeFilter 
     * @param properties 
     * @return uint32_t 
     */
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /**
     * @brief Copies one buffer to another.
     * 
     * @param srcBuffer 
     * @param dstBuffer 
     * @param size Size of the buffer.
     */
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    /**
     * @brief Copies buffer to image.
     * 
     * @param buffer 
     * @param image 
     * @param dims 
     * @param aspectMask 
     */
    void copyBufferToImage(VkBuffer buffer, VkImage image, glm::vec2 dims,
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

    void copyImageToImage(std::shared_ptr<Image> src, std::shared_ptr<Image> dst,
        VkCommandBuffer commandBuffer);
    
    /**
     * @brief Transitions image from one layout to another.
     * 
     * @param image 
     * @param format 
     * @param oldL 
     * @param newL 
     * @param aspectMask 
     */
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldL, VkImageLayout newL,
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

    /**
     * @brief Creates a image view of the image.
     * 
     * @param image 
     * @param format 
     * @param aspectMask 
     * @return VkImageView 
     */
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask);

    /**
     * @brief Creates image barrier.
     * 
     * @param commandBuffer Command buffer to be used.
     * @param src Source access flags.
     * @param dst Destination access flags.
     * @param oldL Old layout.
     * @param newL New layout
     * @param image
     * @param imgAspectFlags 
     * @param srcStage Source pipeline stage. 
     * @param dstStage Destination pipeline stage.
     */
    void createImageBarrier(VkCommandBuffer commandBuffer, VkAccessFlags src, VkAccessFlags dst, VkImageLayout oldL, VkImageLayout newL,
        VkImage image, VkImageAspectFlags imgAspectFlags, VkPipelineStageFlags srcStage,
        VkPipelineStageFlags dstStage);

    /**
     * @brief Creates a memory barrier.
     * 
     * @param commandBuffer 
     * @param src Source access flags.
     * @param dst Destination access flags.
     * @param srcStage 
     * @param dstStage 
     */
    void createMemoryBarrier(VkCommandBuffer commandBuffer, VkAccessFlags src, VkAccessFlags dst, VkPipelineStageFlags srcStage,
        VkPipelineStageFlags dstStage);

    /**
     * @brief Initializes command buffer for single use.
     * 
     * @param commandBuffer 
     */
    void beginSingleCommands(VkCommandBuffer& commandBuffer);

    /**
     * @brief Ends the single use command buffer.
     * 
     * @param commandBuffer 
     */
    void endSingleCommands(VkCommandBuffer& commandBuffer);
private:
    // Create methods.
    void createInstance();
    void createSurface(std::shared_ptr<Window> window);
    void createLogicalDevice();
    void createCommandPool();
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
        VkFormatFeatureFlags features);

    // void createCommandPool();

    void pickPhysicalDevice();

    std::vector<const char*> getRequiredExtensions();

    bool checkValidationLayerSupport();

    // Debug functions
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();

    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device;
    VkCommandPool m_commandPool;
    VkPhysicalDeviceFeatures m_features;
    VkFormat m_depthFormat;

    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkQueue m_computeQueue;

    QueueFamilyIndices m_familyIndices;

    bool m_enableValidationLayers = true;

    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation" 
    };

    const std::vector<const char*> m_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
        // VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
    };

    // Debug
    VkDebugUtilsMessengerEXT m_debugMessenger;
};

}