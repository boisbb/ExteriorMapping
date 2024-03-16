#include "descriptors/Pool.h"

namespace vke
{

DescriptorPool::DescriptorPool(std::shared_ptr<Device> device, uint32_t maxSets, VkDescriptorPoolCreateFlags flags,
    std::vector<VkDescriptorPoolSize> poolSizes)
    : m_device(device)
{
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.flags = flags;
    poolInfo.maxSets = maxSets;

    if (vkCreateDescriptorPool(m_device->getVkDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

DescriptorPool::~DescriptorPool()
{
}

void DescriptorPool::destroyVkResources()
{
    vkDestroyDescriptorPool(m_device->getVkDevice(), m_descriptorPool, nullptr);
}

void DescriptorPool::allocateSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptorSet,
                                 const void *pNext) const
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.pNext = pNext;

    if (vkAllocateDescriptorSets(m_device->getVkDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
}
}

namespace vke
{

VkDescriptorPoolSize createPoolSize(VkDescriptorType type, uint32_t descriptorCount)
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = type;
    poolSize.descriptorCount = descriptorCount;

    return poolSize;
}

}