#include "Image.h"
#include "Device.h"

#include <cstring>

namespace vke
{

Image::Image(std::shared_ptr<Device> device, glm::vec2 dims, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout initialLayout)
    : m_dims(dims), m_device(device), m_format(format), m_tiling(tiling),
    m_usage(usage), m_properties(properties), m_layout(initialLayout)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = dims.x;
    imageInfo.extent.height = dims.y;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = m_layout;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_device->getVkDevice(), &imageInfo, nullptr, &m_image) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed creating image.");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device->getVkDevice(), m_image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_device->findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device->getVkDevice(), &allocInfo, nullptr, &m_imageMemory) != VK_SUCCESS)
        throw std::runtime_error("Failed allocating image memory");

    vkBindImageMemory(m_device->getVkDevice(), m_image, m_imageMemory, 0);
}

Image::~Image()
{
}

void Image::transitionImageLayout(VkImageLayout oldL, VkImageLayout newL, VkImageAspectFlags aspectMask)
{
    m_device->transitionImageLayout(m_image, m_format, oldL, newL, aspectMask);
    m_layout = newL;
}

VkImageView Image::createImageView(VkImageAspectFlags aspectMask)
{
    return m_device->createImageView(m_image, m_format, aspectMask);
}

VkImage Image::getVkImage() const
{
    return m_image;
}

VkImageLayout Image::getVkImageLayout() const
{
    return m_layout;
}

VkFormat Image::getVkFormat() const
{
    return m_format;
}

}