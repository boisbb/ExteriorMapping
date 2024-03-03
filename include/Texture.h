/**
 * @file Texture.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "Device.h"

#include <vulkan/vulkan.h>

#include "glm_include_unified.h"

namespace vke
{

class Image;
class Sampler;
class DescriptorSet;

class Texture
{
public:
    /**
     * @brief Construct a new Texture object.
     * 
     * @param device Device for the texture.
     * @param pixels Pixel data.
     * @param dims Dimensions.
     * @param channels Number of channels.
     * @param format Format of the texture.
     */
    Texture(std::shared_ptr<Device> device, unsigned char* pixels, glm::vec2 dims, int channels = 4,
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);
    ~Texture();

    std::shared_ptr<Sampler> getSampler() const;
    std::shared_ptr<Image> getImage() const;
    VkDescriptorImageInfo getInfo() const;

private:
    std::shared_ptr<Device> m_device;
    std::shared_ptr<Image> m_image;
    std::shared_ptr<Sampler> m_sampler;

    std::vector<std::shared_ptr<DescriptorSet>> m_descriptorSets;

    VkImageView m_imageView;
};

}