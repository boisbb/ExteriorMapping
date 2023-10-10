#pragma once

#include <vulkan/vulkan.h>

#include <unordered_map>

#include "Device.h"
#include "SwapChain.h"
#include "Texture.h"

namespace vke
{

class Renderer
{
public:
    Renderer(std::shared_ptr<Device> device, std::shared_ptr<Window> window);
    ~Renderer();

    std::shared_ptr<SwapChain> getSwapChain() const;
    VkCommandBuffer getCommandBuffer(int id) const;
    std::unordered_map<std::string, std::shared_ptr<Texture>>& getTextureMap();


    void beginRenderPass(int currentFrame, uint32_t imageIndex);
    void endRenderPass(int currentFrame);
private:
    void createCommandBuffers();

    std::shared_ptr<Device> m_device;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<SwapChain> m_swapChain;

    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureMap;

    std::vector<VkCommandBuffer> m_commandBuffers;
};

}