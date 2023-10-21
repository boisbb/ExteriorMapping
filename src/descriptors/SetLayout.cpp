#include "descriptors/SetLayout.h"

#include <iostream>

namespace vke
{

DescriptorSetLayout::DescriptorSetLayout(std::shared_ptr<Device> device, std::vector<VkDescriptorSetLayoutBinding> vkbindings,
    VkDescriptorSetLayoutCreateFlags flags, const void* pNext)
    : m_device(device)
{
    for (auto vkbinding : vkbindings)
        m_vkbindings[vkbinding.binding] = vkbinding;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = vkbindings.size();
    layoutInfo.pBindings = vkbindings.data();
    layoutInfo.flags = flags;
    layoutInfo.pNext = pNext;

    if (vkCreateDescriptorSetLayout(m_device->getVkDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}

VkDescriptorSetLayoutBinding DescriptorSetLayout::getBinding(uint32_t binding)
{
    return m_vkbindings[binding];
}

VkDescriptorSetLayout DescriptorSetLayout::getLayout()
{
    return m_descriptorSetLayout;
}

}

namespace vke
{

VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType,
    uint32_t descriptorCount, VkShaderStageFlags stageFlags, VkSampler* pImmutableSamplers)
{
    VkDescriptorSetLayoutBinding setLayoutBinding{};
    setLayoutBinding.binding = binding;
    setLayoutBinding.descriptorType = descriptorType;
    setLayoutBinding.descriptorCount = descriptorCount;
    setLayoutBinding.stageFlags = stageFlags;
    setLayoutBinding.pImmutableSamplers = pImmutableSamplers;

    return setLayoutBinding;
}

}