#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"
#include "descriptors/SetLayout.h"
#include "descriptors/Pool.h"

namespace vke
{

class DescriptorSet
{
public:
    DescriptorSet(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
        std::shared_ptr<DescriptorPool> descriptorPool, std::vector<uint32_t> binding, std::vector<VkDescriptorBufferInfo> bufferInfo);
    ~DescriptorSet();

    VkDescriptorSet getDescriptorSet();
private:
    std::shared_ptr<Device> m_device;
    std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;
    std::shared_ptr<DescriptorPool> m_descriptorPool;

    VkDescriptorSet m_descriptorSet;
};

}