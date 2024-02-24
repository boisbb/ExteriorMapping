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

#include "glm_include_unified.h"

namespace vke
{

class Window
{
public:
    Window(int width = 800, int height = 600, bool visible = true);
    ~Window();
    
    void createWindowSurface(VkInstance instance, VkSurfaceKHR& surface);
    void createWindowSurface(VkInstance instance);

    VkExtent2D getExtent();
    glm::vec2 getResolution();
    GLFWwindow* getWindow();
    bool resized() const;
    VkSurfaceKHR getSurface() const;

    void setResized(bool resized);
    void setVisible(bool visible);

private:
    int m_width;
    int m_height;
    bool m_visible;

    bool m_windowResized = false;

    GLFWwindow* m_window;

    VkSurfaceKHR m_surface;
};

}
