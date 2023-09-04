#include "descriptors/Set.h"

namespace vke
{

DescriptorSet::DescriptorSet(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
    std::shared_ptr<DescriptorPool> descriptorPool, uint32_t binding, VkDescriptorBufferInfo *bufferInfo)
    : m_device(device), m_descriptorSetLayout(descriptorSetLayout), m_descriptorPool(descriptorPool)
{
    VkDescriptorType dType = m_descriptorSetLayout->getBinding(binding).descriptorType;
    
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.descriptorType = dType;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    m_descriptorPool->allocateSet(m_descriptorSetLayout->getLayout(), m_descriptorSet);
    
    descriptorWrite.dstSet = m_descriptorSet;

    vkUpdateDescriptorSets(m_device->getVkDevice(), 1, &descriptorWrite, 0, nullptr);
}

DescriptorSet::~DescriptorSet()
{
}

VkDescriptorSet DescriptorSet::getDescriptorSet()
{
    return m_descriptorSet;
}
}
