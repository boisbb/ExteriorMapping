/**
 * @file GraphicsPipeline.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-05-13
 * 
 * 
 */

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
        std::string fragFile, std::vector<VkDescriptorSetLayout> graphicsSetLayouts, 
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, bool cullBack = true,
        bool vertexAttribs = true);
    ~GraphicsPipeline();

    void create(VkRenderPass renderPass, std::string vertFile, std::string fragFile,
        std::vector<VkDescriptorSetLayout> graphicsSetLayouts, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        bool cullBack = true, bool vertexAttribs = true);
    
    void bind(VkCommandBuffer commandBuffer) const override;

    VkRenderPass getRenderPass() const;

private:
    VkRenderPass m_renderPass;
};

}