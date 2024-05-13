/**
 * @file Set.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-05-13
 * 
 * 
 */

#include "descriptors/Set.h"

namespace vke
{

DescriptorSet::DescriptorSet(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
    std::shared_ptr<DescriptorPool> descriptorPool, const void* pNext)
    : m_device(device), m_descriptorSetLayout(descriptorSetLayout), m_descriptorPool(descriptorPool)
{
    m_descriptorPool->allocateSet(m_descriptorSetLayout->getLayout(), m_descriptorSet, pNext);
}

DescriptorSet::~DescriptorSet()
{
}


void DescriptorSet::updateBuffers(std::vector<uint32_t> binding, std::vector<VkDescriptorBufferInfo> bufferInfo,
        uint32_t dstArrayElement)
{
    for (int i = 0; i < binding.size(); i++)
    {
        VkDescriptorType dType = m_descriptorSetLayout->getBinding(binding[i]).descriptorType;
        
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstBinding = binding[i];
        descriptorWrite.descriptorType = dType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.dstArrayElement = dstArrayElement;
        descriptorWrite.pBufferInfo = &bufferInfo[i];
        descriptorWrite.pImageInfo = nullptr;
        descriptorWrite.pTexelBufferView = nullptr;
        descriptorWrite.dstSet = m_descriptorSet;

        vkUpdateDescriptorSets(m_device->getVkDevice(), 1, &descriptorWrite, 0, nullptr);
    }

}

void DescriptorSet::updateImages(std::vector<uint32_t> binding, std::vector<VkDescriptorImageInfo> imageInfo,
        uint32_t dstArrayElement)
{
    for (int i = 0; i < binding.size(); i++)
    {

        VkDescriptorType dType = m_descriptorSetLayout->getBinding(binding[i]).descriptorType;
        
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstBinding = binding[i];
        descriptorWrite.descriptorType = dType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.dstArrayElement = dstArrayElement;
        descriptorWrite.pImageInfo = &imageInfo[i];
        descriptorWrite.pTexelBufferView = nullptr;
        descriptorWrite.dstSet = m_descriptorSet;

        vkUpdateDescriptorSets(m_device->getVkDevice(), 1, &descriptorWrite, 0, nullptr);
    }

}

VkDescriptorSet DescriptorSet::getDescriptorSet()
{
    return m_descriptorSet;
}
}
