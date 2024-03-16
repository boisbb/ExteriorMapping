#pragma once

#include <vulkan/vulkan.h>

#include <memory>

namespace vke
{

class Device;


class Buffer
{
public:
    Buffer(std::shared_ptr<Device> device, VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties);
    ~Buffer();
    
    void destroyVkResources();

    VkDescriptorBufferInfo getInfo() const;
    void* getMapped() const;
    VkDeviceSize getSize() const;
    VkBuffer getVkBuffer() const;

    void map();
    void unmap();

    void copyMapped(void* data, size_t size);
    
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