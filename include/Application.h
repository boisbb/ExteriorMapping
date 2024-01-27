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

// GLM
#include "glm_include_unified.h"

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
#include "View.h"
#include "utils/Config.h"

namespace vke
{

class Application
{
public:
    Application();
    
    void run();

    bool framebufferResized = false;
private:

    struct ViewColInfo {
        int rowId;
        int viewId;
        bool remove;
    };

    void init();
    void draw();
    void consumeInput();
    void initImgui();
    void renderImgui(int lastFps);
    void cleanup();

    void addConfigViews();
    void addViewColumn(int rowId, int rowViewStartId);
    void removeViewColumn(int rowId, int rowViewStartId);
    void addViewRow();
    void removeViewRow();
    void resizeAllViews();

    void createScene();
    void createModels();

    // Test
    void mainCameraTestRays();
    //

    std::shared_ptr<Window> m_window;
    std::shared_ptr<Device> m_device;
    std::shared_ptr<Renderer> m_renderer;
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<Model> m_light;
    std::vector<std::shared_ptr<Model>> m_models;
    
    std::shared_ptr<View> m_mainView;
    std::vector<std::shared_ptr<View>> m_views;

    std::shared_ptr<Model> m_cameraCube;
    
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    std::vector<uint32_t> m_viewRowColumns;

    bool m_showCameraGeometry;

    utils::Config m_config;
};

}