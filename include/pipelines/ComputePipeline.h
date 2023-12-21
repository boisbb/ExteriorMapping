#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"
#include "Pipeline.h"

namespace vke
{

class ComputePipeline : public Pipeline
{
public:
    ComputePipeline(std::shared_ptr<Device> device, std::string compFile,
        std::vector<VkDescriptorSetLayout> computeSetLayouts);
    ~ComputePipeline();

    void create(std::string compFile, std::vector<VkDescriptorSetLayout> computeSetLayouts);

    void bind(VkCommandBuffer commandBuffer) const override;
};

}