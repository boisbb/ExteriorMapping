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

#include "Window.h"
#include "Device.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "Buffer.h"
#include "Renderer.h"
#include "Model.h"
#include "Mesh.h"
#include "descriptors/SetLayout.h"
#include "descriptors/Pool.h"
#include "descriptors/Set.h"

#define WIDTH 800
#define HEIGHT 600

namespace vke
{

class Application
{
public:
    void run();

    bool framebufferResized = false;
private:
    void init();
    void setupVulkan();

    void drawFrame();

    // void createVertexBuffer();
    // void createIndexBuffer();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void updateUniformBuffer(uint32_t currentImage);

    std::shared_ptr<Window> m_window;

    std::shared_ptr<Device> m_device;

    std::shared_ptr<Pipeline> m_pipeline;

    std::shared_ptr<Renderer> m_renderer;

    std::shared_ptr<Model> m_model;

    std::vector<std::shared_ptr<DescriptorSet>> m_dSets;
    std::vector<std::unique_ptr<Buffer>> ubos;

    VkPipelineLayout m_vkPipelineLayout;

};

}