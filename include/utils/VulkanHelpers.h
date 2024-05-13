/**
 * @file VulkanHelpers.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * 
 */

#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"
#include "utils/Structs.h"

namespace vke::utils
{

/**
 * @brief Find queue families for the surface and physical device.
 * 
 * @param device 
 * @param surface 
 * @return vke::QueueFamilyIndices 
 */
vke::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

/**
 * @brief Query the swapchain support details for physical device and surface.
 * 
 * @param device 
 * @param surface 
 * @return SwapChainSupportDetails 
 */
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

/**
 * @brief Check whether the device is suitable for rendering.
 * 
 * @param device 
 * @param surface 
 * @param deviceExtensions 
 * @return true 
 * @return false 
 */
bool checkDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*> deviceExtensions);

/**
 * @brief Check whether the needed extensions are supported.
 * 
 * @param device 
 * @param deviceExtensions 
 * @return true 
 * @return false 
 */
bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*> deviceExtensions);

}
