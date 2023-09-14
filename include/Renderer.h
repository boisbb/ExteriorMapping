#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"
#include "SwapChain.h"

namespace vke
{

class Renderer
{
public:
    Renderer(std::shared_ptr<Device> device, std::shared_ptr<Window> window);
    ~Renderer();

    std::shared_ptr<SwapChain> getSwapChain() const;
    VkCommandBuffer getCommandBuffer(int id) const;

    void beginRenderPass(int currentFrame, uint32_t imageIndex);
    void endRenderPass(int currentFrame);
private:
    void createCommandBuffers();

    std::shared_ptr<Device> m_device;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<SwapChain> m_swapChain;

    std::vector<VkCommandBuffer> m_commandBuffers;
};

}