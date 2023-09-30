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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Camera.h"

namespace vke::utils
{

void consumeDeviceInput(GLFWwindow* window, std::shared_ptr<Camera>& camera)
{
    static bool firstClick = true;
    static double prevMouseX = 0;
    static double prevMouseY = 0;

    glm::vec3 eye;
    glm::vec3 up;
    glm::vec3 viewDir;
    float speed;

    camera->getCameraInfo(eye, up, viewDir, speed);

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


    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glm::vec2 resolution;
        float sensitivity = 0.f;

        camera->getCameraRotateInfo(resolution, sensitivity);
        
        if (firstClick)
        {
            glfwSetCursorPos(window, resolution.x / 2, resolution.y / 2);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            firstClick = false;
        }
        else
        {
            double mouseX;
            double mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            float rotx = sensitivity * (float)(mouseY - (resolution.y / 2)) / resolution.y;
            float roty = sensitivity * (float)(mouseX - (resolution.x / 2)) / resolution.x;

            glm::vec3 newViewDir = glm::rotate(viewDir, glm::radians(-rotx), glm::normalize(glm::cross(viewDir, up)));

            if (!((glm::angle(newViewDir, up) <= glm::radians(0.5f)) || (glm::angle(newViewDir, -up) <= glm::radians(5.0f))))
            {
                viewDir = newViewDir;
                }

            viewDir = glm::rotate(viewDir, glm::radians(-roty), up);

            glfwSetCursorPos(window, (resolution.x / 2), (resolution.y / 2));
        }

    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick = true;
    }

    camera->setCameraInfo(eye, up, viewDir, speed);
}


}
