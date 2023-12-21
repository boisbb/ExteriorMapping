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

    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;

    virtual void bind(VkCommandBuffer commandBuffer) const = 0;

protected:
    VkShaderModule createShaderModule(const std::vector<char>& code);

    std::shared_ptr<Device> m_device;

    VkPipeline m_pipeline;
    VkPipelineLayout m_pipelineLayout;
};

}