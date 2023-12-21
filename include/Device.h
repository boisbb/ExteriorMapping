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

class Device
{
public:
    Device(std::shared_ptr<Window> window);
    ~Device();

    QueueFamilyIndices getQueueFamilies();
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

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBufferToImage(VkBuffer buffer, VkImage image, glm::vec2 dims);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldL, VkImageLayout newL);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask);

    void beginSingleCommands(VkCommandBuffer& commandBuffer);
    void endSingleCommands(VkCommandBuffer& commandBuffer);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
private:
    void createInstance();
    void createSurface();
    void createLogicalDevice();
    void createCommandPool();

    // void createCommandPool();

    void pickPhysicalDevice();

    // Get functions
    std::vector<const char*> getRequiredExtensions();

    // Check functions
    bool checkValidationLayerSupport();
    bool checkDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    // Find functions
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    // Debug functions
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();

    std::shared_ptr<Window> m_window;

    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device;
    VkCommandPool m_commandPool;
    VkPhysicalDeviceFeatures m_features;

    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkQueue m_computeQueue;

    bool m_enableValidationLayers = true;

    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation" 
    };

    const std::vector<const char*> m_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        // VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
    };

    // Debug
    VkDebugUtilsMessengerEXT m_debugMessenger;
};

}