#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"

namespace vke
{

class Pipeline
{
public:
    Pipeline(std::shared_ptr<Device> device, VkRenderPass renderPass, std::string vertFile,
        std::string fragFile, std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
    ~Pipeline();

    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;

    void bind(VkCommandBuffer commandBuffer);

private:
    VkShaderModule createShaderModule(const std::vector<char>& code);

    std::shared_ptr<Device> m_device;

    VkPipeline m_graphicsPipeline;
    VkPipelineLayout m_pipelineLayout;
};

}