#pragma once

#include <iostream>
#include <memory>

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

// glm
#include "glm_include_unified.h"

#include "View.h"
#include "Camera.h"

namespace vke::utils
{

void consumeDeviceInput(GLFWwindow* window, glm::vec2 framebufferRatio, std::vector<std::shared_ptr<View>> views)
{
    double mouseX;
    double mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    std::shared_ptr<Camera> camera = views[0]->getCamera();
    std::shared_ptr<View> view = views[0];

    glm::vec2 offset;
    glm::vec2 resolution;

    for (int i = 0; i < views.size(); i++)
    {
        auto v = views[i];
        offset = v->getViewportStart() * framebufferRatio;
        resolution = v->getResolution() * framebufferRatio;

        if ((mouseX >= offset.x && mouseX < offset.x + resolution.x)
         && (mouseY >= offset.y && mouseY < offset.y + resolution.y))
        {
            camera = v->getCamera();
            view = v;
        }
    }

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
        float sensitivity = camera->getSensitivity();
        offset = view->getViewportStart() * framebufferRatio;
        resolution = view->getResolution() * framebufferRatio;
        
        if (firstClick)
        {
            glfwSetCursorPos(window, std::round(offset.x + (resolution.x / 2)), std::round(offset.y + (resolution.y / 2)));
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            firstClick = false;
        }
        else
        {
            float rotx = sensitivity * (float)(mouseY - std::round((offset.y + (resolution.y / 2)))) / resolution.y;
            float roty = sensitivity * (float)(mouseX - std::round((offset.x + (resolution.x / 2)))) / resolution.x;

            float t = (float)((offset.y + (resolution.y / 2)));
            float t2 = (float)((offset.x + (resolution.x / 2)));

            glm::vec3 newViewDir = glm::rotate(viewDir, glm::radians(-rotx), glm::normalize(glm::cross(viewDir, up)));

            if (!((glm::angle(newViewDir, up) <= glm::radians(0.5f)) || (glm::angle(newViewDir, -up) <= glm::radians(5.0f))))
            {
                viewDir = newViewDir;
                }

            viewDir = glm::rotate(viewDir, glm::radians(-roty), up);

            glfwSetCursorPos(window, std::round(offset.x + (resolution.x / 2)), std::round(offset.y + (resolution.y / 2)));
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
