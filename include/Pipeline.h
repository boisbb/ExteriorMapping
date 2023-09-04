#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"

namespace vke
{

class Pipeline
{
public:
    Pipeline(std::shared_ptr<Device> device, VkRenderPass renderPass, std::string vertFile,
        std::string fragFile, VkPipelineLayout pipelineLayout);
    ~Pipeline();

    VkPipeline getPipeline();

private:
    VkShaderModule createShaderModule(const std::vector<char>& code);

    std::shared_ptr<Device> m_device;

    VkPipeline m_graphicsPipeline;
};

}