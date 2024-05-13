/**
 * @file RenderPass.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * 
 */

#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"

namespace vke
{

class RenderPass
{
public:
    /**
     * @brief Construct a new Render Pass object.
     * 
     * @param device Device for the render pass.
     * @param colorFormat Format of the color attachement.
     * @param depthFormat Format of the depth attachement.
     * @param offscreen Whether or not the render pass is offscreen.
     */
    RenderPass(std::shared_ptr<Device> device, VkFormat colorFormat, VkFormat depthFormat,
        bool offscreen = false);
    ~RenderPass();

    void destroyVkResources();

    VkRenderPass getRenderPass() const;
    bool isOffscreen() const;

private:
    void createRenderPass();

    std::shared_ptr<Device> m_device;

    VkFormat m_colorFormat;
    VkFormat m_depthFormat;

    bool m_offscreen;

    VkRenderPass m_renderPass;
};

}