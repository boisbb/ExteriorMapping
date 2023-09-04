#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"

namespace vke
{

class Buffer
{
public:
    Buffer(std::shared_ptr<Device> device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~Buffer();

    VkDescriptorBufferInfo getInfo();

    void* getMapped();

    void map();
    void unmap();
private:
    std::shared_ptr<Device> m_device;

    VkBuffer m_buffer;
    VkDeviceMemory m_bufferMemory;
    VkDeviceSize m_size;
    VkBufferUsageFlags m_usage;
    VkMemoryPropertyFlags m_properties;

    void* m_memoryMapped;
};

}