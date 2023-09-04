#pragma once

// GLFW
#define VK_USE_PLATFORM_XCB_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#include "Application.h"
#include "Window.h"

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto wind = reinterpret_cast<vke::Window*>(glfwGetWindowUserPointer(window));
    wind->framebufferResized = true;
}

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