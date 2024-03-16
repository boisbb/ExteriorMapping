/**
 * @file Window.h
 * @author Boris Burkalo (xburka00)
 * @brief 
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

#include "glm_include_unified.h"

namespace vke
{

class Window
{
public:
    /**
     * @brief Construct a new Window object.
     * 
     * @param width 
     * @param height 
     * @param visible If the window is visible.
     */
    Window(int width = 800, int height = 600, bool visible = true);
    ~Window();

    void destroyVkResources(VkInstance instance);
    
    /**
     * @brief Create a Window Surface and return it.
     * 
     * @param instance Vulkan instance. 
     * @param surface Vulkan surface.
     */
    void createWindowSurface(VkInstance instance, VkSurfaceKHR& surface);

    /**
     * @brief Create a Window Surface object.
     * 
     * @param instance 
     */
    void createWindowSurface(VkInstance instance);

    // Getters
    VkExtent2D getExtent();
    glm::vec2 getResolution();
    GLFWwindow* getWindow();
    bool resized() const;
    VkSurfaceKHR getSurface() const;
    bool getVisible() const;
    
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
