#pragma once

#include <iostream>
#include <memory>

// GLFW
#define VK_USE_PLATFORM_XCB_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

namespace vke::utils
{

void consumeDeviceInput(GLFWwindow* window, std::shared_ptr<Camera>& camera)
{
    glm::vec3 eye;
    glm::vec3 center;
    glm::vec3 up;
    glm::vec3 viewDir;
    float speed;

    camera->getCameraInfo(eye, center, up, viewDir, speed);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        eye += speed * viewDir;
    }
    
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        eye += speed * -glm::normalize(glm::cross(viewDir, up));
    }
    
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        eye += speed * -viewDir;
    }
    
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        eye += speed * glm::normalize(glm::cross(viewDir, up));
    }
    
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        eye += speed * up;
    }
    
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        eye += speed * -up;
    }

    camera->setCameraInfo(eye, center, up, viewDir, speed);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        
    }

    
}


}
