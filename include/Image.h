/**
 * @file Image.h
 * @author Boris Burkalo (xburka00)
 * @date 2024-03-03
 * 
 * 
 */

#pragma once

// Vulkan
#include <vulkan/vulkan.h>

// glm
#include "glm_include_unified.h"

// std
#include <memory>

namespace vke
{

class Device;

class Image
{
public:
    /**
     * @brief Construct a new Image object
     * 
     * @param device 
     * @param dims 
     * @param format 
     * @param tiling 
     * @param usage 
     * @param properties 
     * @param initialLayout 
     */
    Image(std::shared_ptr<Device> device, glm::vec2 dims, VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);
    ~Image();

    void destroyVkResources();

    /**
     * @brief Transition image from one layout to another.
     * 
     * @param oldL 
     * @param newL 
     * @param aspectMask 
     */
    void transitionImageLayout(VkImageLayout oldL, VkImageLayout newL, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

    /**
     * @brief Create an Image view.
     * 
     * @param aspectMask 
     * @return VkImageView 
     */
    VkImageView createImageView(VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

    void map();

    void unmap();

    // Getters
    VkImage getVkImage() const;
    VkImageLayout getVkImageLayout() const;
    VkFormat getVkFormat() const;
    glm::vec2 getDims() const;
    void* getMapped();
    
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