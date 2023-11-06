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
#define VK_USE_PLATFORM_XCB_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ImGui
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include "Window.h"
#include "Device.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "Renderer.h"
#include "Model.h"
#include "Scene.h"
#include "Camera.h"

namespace vke
{

class Application
{
public:
    Application();
    
    void run();

    bool framebufferResized = false;
private:
    void init();
    void draw();
    void consumeInput();
    void initImgui();
    void renderImgui();
    void cleanup();

    std::shared_ptr<Window> m_window;
    std::shared_ptr<Device> m_device;
    std::shared_ptr<Renderer> m_renderer;
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<Model> m_light;
    std::vector<std::shared_ptr<Model>> m_models;
    std::shared_ptr<Camera> m_camera;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
};

}