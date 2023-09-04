#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"

namespace vke
{

class DescriptorPool
{
public:
    DescriptorPool(std::shared_ptr<Device> device, uint32_t maxSets, VkDescriptorPoolCreateFlags flags,
        std::vector<VkDescriptorPoolSize> poolSizes);
    ~DescriptorPool();

    void allocateSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet) const;
private:
    std::shared_ptr<Device> m_device;

    VkDescriptorPool m_descriptorPool;
};

}