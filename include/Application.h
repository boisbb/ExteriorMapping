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
#include "Texture.h"
#include "Sampler.h"
#include "Mesh.h"
#include "Camera.h"
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
    Application();
    
    void run();

    bool framebufferResized = false;
private:
    void init();
    void draw();
    void cleanup();

    void renderFrame();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void updateUniformBuffer(uint32_t currentImage);

    uint32_t m_currentFrame = 0;

    std::shared_ptr<Window> m_window;
    std::shared_ptr<Device> m_device;
    std::shared_ptr<Pipeline> m_pipeline;
    std::shared_ptr<Renderer> m_renderer;

    std::shared_ptr<Model> m_model;
    std::shared_ptr<Texture> m_texture;
    std::shared_ptr<Texture> m_texture2;
    std::shared_ptr<Sampler> m_sampler;

    std::shared_ptr<Camera> m_camera;

    std::vector<std::shared_ptr<DescriptorSet>> m_dSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_textSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_textSets2;

    std::vector<std::unique_ptr<Buffer>> ubos;
    std::vector<std::unique_ptr<Buffer>> sbos;
    
    VkPipelineLayout m_vkPipelineLayout;

};

}