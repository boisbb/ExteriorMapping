#pragma once

#include <vulkan/vulkan.h>

#include "glm_include_unified.h"

#include <memory>

namespace vke
{

class Device;

class Image
{
public:
    Image(std::shared_ptr<Device> device, glm::vec2 dims, VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);
    ~Image();

    void transitionImageLayout(VkImageLayout oldL, VkImageLayout newL, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
    VkImageView createImageView(VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

    VkImage getVkImage() const;
    VkImageLayout getVkImageLayout() const;
    VkFormat getVkFormat() const;

private:
    glm::vec2 m_dims;

    std::shared_ptr<Device> m_device;

    VkImage m_image;
    VkDeviceMemory m_imageMemory;
    VkFormat m_format;
    VkImageTiling m_tiling;
    VkImageUsageFlags m_usage;
    VkMemoryPropertyFlags m_properties;
    VkImageLayout m_layout;

    void* m_memoryMapped;
};

}