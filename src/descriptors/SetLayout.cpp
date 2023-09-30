#include "descriptors/SetLayout.h"

#include <iostream>

namespace vke
{

DescriptorSetLayout::DescriptorSetLayout(std::shared_ptr<Device> device, std::vector<VkDescriptorSetLayoutBinding> vkbindings)
    : m_device(device)
{
    for (auto vkbinding : vkbindings)
        m_vkbindings[vkbinding.binding] = vkbinding;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = vkbindings.size();
    layoutInfo.pBindings = vkbindings.data();

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