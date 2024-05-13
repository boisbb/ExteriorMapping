/**
 * @file SetLayout.h
 * @author Boris Burkalo (xburka00)
 * @brief Functionality for the descriptor set layout.
 * @date 2024-05-13
 * 
 * 
 */

#pragma once

#include <vector>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "Device.h"

namespace vke
{

VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType,
    uint32_t descriptorCount, VkShaderStageFlags stageFlags, VkSampler* pImmutableSamplers = nullptr);

class DescriptorSetLayout
{
public:
    DescriptorSetLayout(std::shared_ptr<Device> device, std::vector<VkDescriptorSetLayoutBinding> vkbindings,
        VkDescriptorSetLayoutCreateFlags flags = 0, const void* pNext = nullptr);
    ~DescriptorSetLayout();

    void destroyVkResources();

    VkDescriptorSetLayoutBinding getBinding(uint32_t binding);
    VkDescriptorSetLayout getLayout();
private:
    std::shared_ptr<Device> m_device;

    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_vkbindings;

    VkDescriptorSetLayout m_descriptorSetLayout;
};

}