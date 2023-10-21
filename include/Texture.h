#pragma once

#include "Device.h"

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

namespace vke
{

class Image;
class Sampler;
class DescriptorSet;

class Texture
{
public:
    Texture(std::shared_ptr<Device> device, unsigned char* pixels, glm::vec2 dims, int channels = 4,
        VkFormat format = VK_FORMAT_R8G8B8_SRGB);
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