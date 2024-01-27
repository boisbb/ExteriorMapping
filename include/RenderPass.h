#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"

namespace vke
{

class RenderPass
{
public:
    RenderPass(std::shared_ptr<Device> device, VkFormat colorFormat, VkFormat depthFormat,
        bool offscreen = false);
    ~RenderPass();

    VkRenderPass getRenderPass() const;

private:
    void createRenderPass();

    std::shared_ptr<Device> m_device;

    VkFormat m_colorFormat;
    VkFormat m_depthFormat;

    bool m_offscreen;

    VkRenderPass m_renderPass;
};

}