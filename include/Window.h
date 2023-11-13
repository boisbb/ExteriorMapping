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
#define VK_USE_PLATFORM_XCB_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#include "utils/Constants.h"

namespace vke
{

class Window
{
public:
    Window(int width = 800, int height = 600);
    ~Window();
    
    void createWindowSurface(VkInstance& instance, VkSurfaceKHR* surface);

    VkExtent2D getExtent();
    GLFWwindow* getWindow();

    bool framebufferResized = false;
private:
    void init();

    void createGlfwWindow();

    int m_width;
    int m_height;

    GLFWwindow* m_window;
};

}
