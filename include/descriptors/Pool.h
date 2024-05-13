/**
 * @file Pool.h
 * @author Boris Burkalo (xburka00)
 * @brief Functionality for the descriptor pool.
 * @date 2024-05-13
 * 
 * 
 */

#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"

namespace vke
{

VkDescriptorPoolSize createPoolSize(VkDescriptorType type, uint32_t descriptorCount);

class DescriptorPool
{
public:
    DescriptorPool(std::shared_ptr<Device> device, uint32_t maxSets, VkDescriptorPoolCreateFlags flags,
        std::vector<VkDescriptorPoolSize> poolSizes);
    ~DescriptorPool();

    void destroyVkResources();

    void allocateSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet,
        const void* pNext = nullptr) const;
private:
    std::shared_ptr<Device> m_device;

    VkDescriptorPool m_descriptorPool;
};

}