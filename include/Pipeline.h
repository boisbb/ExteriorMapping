/**
 * @file Pipeline.h
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

class Pipeline
{
public:
    Pipeline(std::shared_ptr<Device> device);
    ~Pipeline();

    void destroyVkResources();

    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;

    virtual void bind(VkCommandBuffer commandBuffer) const = 0;

protected:
    /**
     * @brief Create a Vulkan shader module object.
     * 
     * @param code Text of the shader.
     * @return VkShaderModule 
     */
    VkShaderModule createShaderModule(const std::vector<char>& code);

    std::shared_ptr<Device> m_device;

    VkPipeline m_pipeline;
    VkPipelineLayout m_pipelineLayout;
};

}