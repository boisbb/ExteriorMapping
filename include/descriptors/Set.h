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
        std::shared_ptr<DescriptorPool> descriptorPool, const void* pNext = nullptr);
    ~DescriptorSet();

    void addBuffers(std::vector<uint32_t> binding, std::vector<VkDescriptorBufferInfo> bufferInfo,
        uint32_t dstArrayElement = 0);
    void addImages(std::vector<uint32_t> binding, std::vector<VkDescriptorImageInfo> imageInfo,
        uint32_t dstArrayElement = 0);

    VkDescriptorSet getDescriptorSet();
private:
    std::shared_ptr<Device> m_device;
    std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;
    std::shared_ptr<DescriptorPool> m_descriptorPool;

    VkDescriptorSet m_descriptorSet;
};

}