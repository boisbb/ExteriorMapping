/**
 * @file Input.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

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

// vke
#include "ViewGrid.h"
#include "Camera.h"

namespace vke::utils
{

/**
 * @brief Consume input for the test pixel.
 * 
 * @param window 
 * @param chosenPixel 
 * @return true
 * @return false 
 */
bool consumeDeviceInputTestPixel(GLFWwindow* window, glm::vec2& chosenPixel)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        double mouseX;
        double mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        chosenPixel = glm::vec2(mouseX, mouseY);

        return true;
    }

    return false;
}

/**
 * @brief Consume device input for camera manipulation.
 * 
 * @param window Window.
 * @param framebufferRatio Ratio of the width and height.
 * @param viewGrid View grid for manipulation.
 * @param manipulateGrid Whether a whole grip is manipulated or not.
 * @param secondaryWindow Whether the window is a secondary window or not.
 * @return true 
 * @return false 
 */
bool consumeDeviceInput(GLFWwindow* window, glm::vec2 framebufferRatio, std::shared_ptr<ViewGrid> viewGrid,
    bool manipulateGrid, bool secondaryWindow = false)
{
    bool changed = false;
    double mouseX;
    double mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    std::vector<std::shared_ptr<View>> views = viewGrid->getViews();
    std::shared_ptr<Camera> camera = views[0]->getCamera();
    std::shared_ptr<View> view = views[0];

    glm::vec3 eye(0.f);
    glm::vec3 up(0.f, 1.f, 0.f);
    glm::vec3 moveViewDir(0.f);
    glm::vec3 rotateViewDir(0.f);
    float speed = 0.f;
    float sensitivity = 0.f;
    glm::vec2 offset(0.f);
    glm::vec2 resolution(0.f);

    if (!manipulateGrid)
    {
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

        if (viewGrid->getByStep() || viewGrid->getByInGridPos())
        {
            viewGrid->getInputInfo(eye, moveViewDir, speed, sensitivity);
            eye = viewGrid->getViewGridPos(view);
            moveViewDir = glm::vec4(view->getCamera()->getViewDir(), 1.f);
            rotateViewDir = view->getCamera()->getViewDir();
        }
        else
        {
            camera->getCameraInfo(eye, up, moveViewDir, speed);
            rotateViewDir = moveViewDir;
        }

        sensitivity = camera->getSensitivity();
        offset = view->getViewportStart() * framebufferRatio;
        resolution = view->getResolution() * framebufferRatio;
    }
    else
    {
        viewGrid->getInputInfo(eye, moveViewDir, speed, sensitivity);
        resolution = viewGrid->getResolution() * framebufferRatio;
        rotateViewDir = moveViewDir;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        eye += speed * moveViewDir;
        changed = true;
    }
    
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        eye += speed * -glm::normalize(glm::cross(moveViewDir, up));
        changed = true;
    }
    
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        eye += speed * -moveViewDir;
        changed = true;
    }
    
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        eye += speed * glm::normalize(glm::cross(moveViewDir, up));
        changed = true;
    }
    
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        eye += speed * up;
        changed = true;
    }
    
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        eye += speed * -up;
        changed = true;
    }

    static bool firstClick[2] = { true, true };
    int windowId = static_cast<int>(secondaryWindow);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (firstClick[windowId])
        {
            glfwSetCursorPos(window, std::round(offset.x + (resolution.x / 2)), std::round(offset.y + (resolution.y / 2)));
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            firstClick[windowId] = false;
        }
        else
        {
            float rotx = sensitivity * (float)(mouseY - std::round((offset.y + (resolution.y / 2)))) / resolution.y;
            float roty = sensitivity * (float)(mouseX - std::round((offset.x + (resolution.x / 2)))) / resolution.x;

            float t = (float)((offset.y + (resolution.y / 2)));
            float t2 = (float)((offset.x + (resolution.x / 2)));

            glm::vec3 newViewDir = glm::rotate(rotateViewDir, glm::radians(-rotx), glm::normalize(glm::cross(rotateViewDir, up)));

            if (!((glm::angle(newViewDir, up) <= glm::radians(0.5f)) || (glm::angle(newViewDir, -up) <= glm::radians(5.0f))))
            {
                rotateViewDir = newViewDir;
            }

            rotateViewDir = glm::rotate(rotateViewDir, glm::radians(-roty), up);

            glfwSetCursorPos(window, std::round(offset.x + (resolution.x / 2)), std::round(offset.y + (resolution.y / 2)));
        }
        changed = true;

    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick[windowId] = true;
    }

    if (!manipulateGrid)
    {
        if (viewGrid->getByStep() || viewGrid->getByInGridPos())
        {
            viewGrid->setViewGridPos(view, eye);
            camera->setViewDir(rotateViewDir);
        }
        else
            camera->setCameraInfo(eye, up, rotateViewDir, speed);
    }
    else
    {
        viewGrid->setInputInfo(eye, rotateViewDir, speed);
    }

    return changed;
}


}
