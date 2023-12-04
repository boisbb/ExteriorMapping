#include "Window.h"
#include "utils/Callbacks.h"

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

void Window::setResized(bool resized)
{
    m_windowResized = resized;
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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, "VulkanApp", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);

    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}

}