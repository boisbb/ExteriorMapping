/**
 * @file Callbacks.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

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
#include "Application.h"
#include "Window.h"

/**
 * @brief Is called whenever window is resized.
 * 
 * @param window 
 * @param width 
 * @param height 
 */
static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto wind = reinterpret_cast<vke::Window*>(glfwGetWindowUserPointer(window));
    wind->setResized(true);
}

static void secondaryWindowCloseCallback(GLFWwindow* window)
{
    auto wind = reinterpret_cast<vke::Window*>(glfwGetWindowUserPointer(window));
    wind->setVisible(false);
    glfwSetWindowShouldClose(window, GLFW_FALSE);
}

/**
 * @brief Validation layer callback.
 * 
 * @param messageSeverity 
 * @param messageType 
 * @param pCallbackData 
 * @param pUserData 
 * @return VKAPI_ATTR 
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        // Message is important enough to show
        std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}