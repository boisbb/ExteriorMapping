#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"

namespace vke
{

class Pipeline
{
public:
    Pipeline(std::shared_ptr<Device> device, VkRenderPass renderPass, std::string vertFile,
        std::string fragFile, std::string compFile, std::vector<VkDescriptorSetLayout> graphicsSetLayouts,
        std::vector<VkDescriptorSetLayout> computeSetLayouts);
    ~Pipeline();

    VkPipeline getGraphicsPipeline() const;
    VkPipelineLayout getGraphicsPipelineLayout() const;
    VkPipeline getComputePipeline() const;
    VkPipelineLayout getComputePipelineLayout() const;

    void bindGraphics(VkCommandBuffer commandBuffer);
    void bindCompute(VkCommandBuffer commandBuffer);

private:
    void createGraphicsPipeline(VkRenderPass renderPass, std::string vertFile, std::string fragFile,
        std::vector<VkDescriptorSetLayout> graphicsSetLayouts);
    void createComputePipeline(std::string compFile,
        std::vector<VkDescriptorSetLayout> computeSetLayouts);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    std::shared_ptr<Device> m_device;

    VkPipeline m_graphicsPipeline;
    VkPipelineLayout m_graphicsPipelineLayout;

    VkPipeline m_computePipeline;
    VkPipelineLayout m_computePipelineLayout;
};

}