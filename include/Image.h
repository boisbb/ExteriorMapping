#pragma once

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <memory>

namespace vke
{

class Device;

class Image
{
public:
    Image(std::shared_ptr<Device> device, glm::vec2 dims, VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    ~Image();

    void transitionImageLayout(VkImageLayout oldL, VkImageLayout newL);
    VkImageView createImageView();

    VkImage getVkImage() const;
private:
    glm::vec2 m_dims;
    
    std::shared_ptr<Device> m_device;

    VkImage m_image;
    VkDeviceMemory m_imageMemory;
    VkFormat m_format;
    VkImageTiling m_tiling;
    VkImageUsageFlags m_usage;
    VkMemoryPropertyFlags m_properties;

    void* m_memoryMapped;
};

}