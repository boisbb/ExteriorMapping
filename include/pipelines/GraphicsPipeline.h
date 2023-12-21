#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"
#include "Pipeline.h"

namespace vke
{

class GraphicsPipeline : public Pipeline
{
public:
    GraphicsPipeline(std::shared_ptr<Device> device, VkRenderPass renderPass, std::string vertFile,
        std::string fragFile, std::vector<VkDescriptorSetLayout> graphicsSetLayouts, bool cullBack = true,
        bool vertexInput = true);
    ~GraphicsPipeline();

    void create(VkRenderPass renderPass, std::string vertFile, std::string fragFile,
        std::vector<VkDescriptorSetLayout> graphicsSetLayouts, bool cullBack = true, 
        bool vertexInput = true);
    
    void bind(VkCommandBuffer commandBuffer) const override;

    VkRenderPass getRenderPass() const;

private:
    VkRenderPass m_renderPass;
};

}