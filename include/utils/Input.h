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

#include "ViewGrid.h"
#include "Camera.h"

namespace vke::utils
{

bool consumeDeviceInput(GLFWwindow* window, glm::vec2 framebufferRatio, std::shared_ptr<ViewGrid> viewGrid,
    bool manipulateGrid)
{
    bool changed = false;
    double mouseX;
    double mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    static bool firstClick = true;
    static double prevMouseX = 0;
    static double prevMouseY = 0;

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

        if (viewGrid->getByStep())
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


    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
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
        firstClick = true;
    }

    // if (false)
    // {
    //     glm::vec4 testEye = glm::vec4(-1.5f + 1.f * testt, 1.f, 0.f, 1.f);
// 
    //     glm::vec3 org = glm::normalize(glm::vec3(0, 0, -1));
    //     glm::vec3 target = glm::normalize(glm::vec3(1, 0, -1));
// 
    //     float angle = glm::acos(glm::dot(org, target));
    //     glm::vec3 axis = glm::normalize(glm::cross(org, target));
    //     
    //     glm::mat4 transform = glm::translate(glm::mat4(1.f), glm::vec3(-10.f, 0.f, 0.f));
// 
    //     transform = glm::rotate(transform, angle, axis);
// 
    //     testEye = transform * glm::vec4(testEye);
// 
    //     eye = testEye;
// 
    //     changed = true;
    // }

    if (!manipulateGrid)
    {
        if (viewGrid->getByStep())
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
