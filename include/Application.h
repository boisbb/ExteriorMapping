/**
 * @file Application.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * 
 */

#pragma once

// std
#include <vector>
#include <iostream>
#include <optional>
#include <fstream>
#include <array>
#include <memory>
#include <thread>

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

// vke
#include "Window.h"
#include "Device.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "Renderer.h"
#include "Model.h"
#include "Scene.h"
#include "Camera.h"
#include "View.h"
#include "ViewGrid.h"
#include "utils/Config.h"

namespace vke
{

class Application
{
public:

    struct Arguments
    {
        enum class EvaluationType
        {
            SAMPLES,
            ONE,
            MSE,
            GT,
            _COUNT
        };

        std::string configFile;
        glm::vec2 windowResolution;
        glm::vec2 novelResolution;
        glm::vec2 viewGridResolution;

        EvaluationType evalType;
        SamplingType samplingType;
        int numberOfSamples = 170;
        bool mseGt = false;
        int numberOfFrames = 1;
    };

    Application(const Arguments& arguments);
    ~Application();
    
    /**
     * @brief Runs the application.
     * 
     */
    void run();

    bool framebufferResized = false;
private:
    /**
     * @brief Initialize the application and rendering resources.
     * 
     */
    void init();

    /**
     * @brief Draw the frames.
     * 
     */
    void draw();

    /**
     * @brief Render the views before the normal render loop starts.
     *        so that the novel view can be rendered right away.
     * 
     */
    void renderViewMatrix(std::shared_ptr<ViewGrid> grid, std::shared_ptr<Framebuffer> framebuffer, bool novelView);

    /**
     * @brief Consumes the user input.
     * 
     * @return true When input came.
     * @return false Otherwise.
     */
    bool consumeInput();

    /**
     * @brief Initialize the ImGui.
     * 
     */
    void initImgui();

    /**
     * @brief Render the ImGui.
     * 
     * @param lastFps Last recorded fps value.
     */
    void renderImgui(int lastFps);

    /**
     * @brief Performs clean up after managed resources.
     * 
     */
    void cleanup();

    /**
     * @brief Create a Main View object.
     * 
     */
    void createMainView();

    /**
     * @brief Create a scene from configuration file.
     * 
     */
    void createScene();

    /**
     * @brief Create a models and put them into scene.
     * 
     */
    void createModels();

    void countFps(int& frames, int& lastFps, double& lastTime);

    void handleGuiInputChanges();

    bool handlePrepareResult(WindowParams &params, glm::vec2& windowResolution,
        glm::vec2& secondaryWindowResolution);

    bool handlePresentResult(WindowParams &params, glm::vec2& windowResolution,
        glm::vec2& secondaryWindowResolution);

    std::shared_ptr<Window> m_window;
    std::shared_ptr<Window> m_secondaryWindow;

    std::shared_ptr<Device> m_device;
    std::shared_ptr<Renderer> m_renderer;
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<Model> m_light;

    std::vector<std::shared_ptr<Model>> m_models;
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    
    std::shared_ptr<ViewGrid> m_novelViewGrid;
    std::shared_ptr<ViewGrid> m_viewGrid;
    std::shared_ptr<Model> m_cameraCube;

    std::shared_ptr<Image> m_viewMatrixScreenshotImage;
    std::shared_ptr<Image> m_novelViewScreenshotImage;
    std::shared_ptr<Image> m_actualViewScreenshotImage;

    // Application argument + evaluate members
    Arguments m_args;
    bool m_terminate = false;
    bool m_evaluate = false;
    int m_evaluateFrames = 0;
    float m_evaluateTotalDuration = 0;
    std::vector<float> m_evalResults;
    int m_evaluateSteps = 0;
    int m_evaluateTotalMseSteps = 0;
    int m_evaluateRotations = 0;
    int m_evaluateSideSteps = 0;
    float m_currentSideStep = MSE_SIDESTEP;
    float m_currentRotation = MSE_ROTATION_ANGLE;
    glm::vec3 m_evaluateOriginalEye;
    glm::vec3 m_evaluateOriginalViewDir;

    // Imgui flags and resources.
    float m_prevTime;
    float m_interval = 0.01f;
    float m_intervalCounter;
    bool m_testPixels = false;
    glm::vec2 m_testedPixel;
    bool m_depthOnly = false;
    bool m_renderNovel = false;
    bool m_novelSecondWindow = false;
    bool m_automaticSampleCount = false;
    int m_numberOfViewsUsed = 4;
    bool m_thresholdDepth = false;
    int m_numberOfRaySamples = 16;
    float m_maxSampleDistance = 1.f;
    bool m_secondWindowChanged = false;
    bool m_showCameraGeometry = false;
    bool m_renderFromViews;
    bool m_manipulateGrid = false;
    int m_changeOffscreenTarget;
    float m_viewsFov;
    float m_mainViewFov;
    SamplingType m_samplingType;
    bool m_reRenderViewMatrix = false;
    bool m_screenshot = false;
    int m_screenshotSaved = 0;
    int m_removeRow = MAX_FRAMES_IN_FLIGHT;
    int m_removeCol = MAX_FRAMES_IN_FLIGHT;
    bool m_pointClouds = false;
    glm::ivec2 m_pointCloudRes = {POINT_CLOUD_WIDTH, POINT_CLOUD_HEIGHT};
    glm::ivec2 m_sampledView = glm::vec2(0,0);

    // threads
    std::thread m_saveImageThread;
    bool m_imagesSaved = true;
    bool m_threadStarted = false;
    
    // Parsed config file.
    utils::Config m_config;

    // ImGui
    VkDescriptorPool m_imguiPool;
};

}