/**
 * @file Window.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "Window.h"
#include "utils/Callbacks.h"

namespace vke
{

Window::Window(int width, int height, bool visible)
    : m_width(width), m_height(height), m_visible(visible)
{
    if (!glfwInit())
        throw std::runtime_error("failed to create glfw context!");
    
    if (!glfwVulkanSupported())
        throw std::runtime_error("vulkan not supported!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, m_visible);

    m_window = glfwCreateWindow(m_width, m_height, "Exterior Mapping", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);

    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Window::destroyVkResources(VkInstance instance)
{
    if (m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR& surface)
{
    if (glfwCreateWindowSurface(instance, m_window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface.");
    }

    m_surface = surface;
}

void Window::createWindowSurface(VkInstance instance)
{
    if (glfwCreateWindowSurface(instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface.");
    }
}

VkExtent2D Window::getExtent()
{
    m_width = 0;
    m_height = 0;
    glfwGetFramebufferSize(m_window, &m_width, &m_height);
    
    while (m_width == 0 || m_height == 0) {
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glfwWaitEvents();
    }

    return {
        static_cast<uint32_t>(m_width),
        static_cast<uint32_t>(m_height)
    };
}

glm::vec2 Window::getResolution()
{
    return glm::vec2(m_width, m_height);
}

GLFWwindow *Window::getWindow()
{
    return m_window;
}

bool Window::resized() const
{
    return m_windowResized;
}

VkSurfaceKHR Window::getSurface() const
{
    return m_surface;
}

bool Window::getVisible() const
{
    return m_visible;
}

void Window::setResized(bool resized)
{
    m_windowResized = resized;
}

void Window::setVisible(bool visible)
{
    m_visible = visible;

    if (visible)
        glfwShowWindow(m_window);
    else
        glfwHideWindow(m_window);
}

}