#include "Buffer.h"

namespace vke
{

Buffer::Buffer(std::shared_ptr<Device> device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    : m_device(device), m_size(size), m_usage(usage), m_properties(properties)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device->getVkDevice(), &bufferInfo, nullptr, &m_buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create vertex buffer");
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device->getVkDevice(), m_buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_device->findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device->getVkDevice(), &allocInfo, nullptr, &m_bufferMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex buffer memory.");
    
    //! TODO: Potentionally add OFFSET
    vkBindBufferMemory(m_device->getVkDevice(), m_buffer, m_bufferMemory, 0);
}

Buffer::~Buffer()
{
}

VkDescriptorBufferInfo Buffer::getInfo()
{
    return VkDescriptorBufferInfo{
        m_buffer,
        0,
        m_size
    };
}

void *Buffer::getMapped()
{
    return m_memoryMapped;
}

void Buffer::map()
{
    // TODO: whole size for now.
    vkMapMemory(m_device->getVkDevice(), m_bufferMemory, 0, VK_WHOLE_SIZE, 0, &m_memoryMapped);
}

void Buffer::unmap()
{
    vkUnmapMemory(m_device->getVkDevice(), m_bufferMemory);
}

}