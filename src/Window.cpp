#include "Window.h"
#include "utils/Constants.h"

namespace vke
{

Window::Window(int width, int height)
    : m_width(width), m_height(height)
{
    init();
}

Window::~Window()
{
}

void Window::createWindowSurface(VkInstance &instance, VkSurfaceKHR *surface)
{
    if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface.");
    }
}

VkExtent2D Window::getExtent()
{
    return {
        static_cast<uint32_t>(WIDTH),
        static_cast<uint32_t>(HEIGHT)
    };
}

GLFWwindow *Window::getWindow()
{
    return m_window;
}

void Window::init()
{
    createGlfwWindow();
}

void Window::createGlfwWindow()
{
    if (!glfwInit())
        throw std::runtime_error("failed to create glfw context!");
    
    if (!glfwVulkanSupported())
        throw std::runtime_error("vulkan not supported!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "VulkanApp", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
}

}