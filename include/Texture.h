#pragma once

#include "Device.h"

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

namespace vke
{

class Image;
class Sampler;

class Texture
{
public:
    Texture(std::shared_ptr<Device> device, unsigned char* pixels, glm::vec2 dims);
    ~Texture();

    void setSampler(std::shared_ptr<Sampler> sampler);
    std::shared_ptr<Sampler> getSampler() const;

private:
    std::shared_ptr<Device> m_device;
    std::shared_ptr<Image> m_image;
    std::shared_ptr<Sampler> m_sampler;

    glm::vec2 m_dims;

    VkImageView m_imageView;
};

}