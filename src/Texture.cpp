#include "Texture.h"
#include "Buffer.h"
#include "Image.h"
#include "Sampler.h"
#include "utils/Constants.h"

namespace vke
{

Texture::Texture(std::shared_ptr<Device> device, unsigned char* pixels, glm::vec2 dims, int channels,
    VkFormat format)
    : m_device(device),
    m_descriptorSets(MAX_FRAMES_IN_FLIGHT)
{
    VkDeviceSize imageSize = dims.x * dims.y * channels;
    
    Buffer stagingBuffer(m_device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    std::cout << "hell" << std::endl;
    stagingBuffer.map();
    stagingBuffer.copyMapped((void*)pixels, static_cast<size_t>(imageSize));
    stagingBuffer.unmap();

    m_image = std::make_shared<Image>(m_device, dims, channels, format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_image->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_device->copyBufferToImage(stagingBuffer.getVkBuffer(), m_image->getVkImage(), dims);

    m_image->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    m_imageView = m_image->createImageView();
}

Texture::~Texture()
{
}


void Texture::setSampler(std::shared_ptr<Sampler> sampler)
{
    m_sampler = sampler;
}

std::shared_ptr<Sampler> Texture::getSampler() const
{
    return m_sampler;
}

std::shared_ptr<Image> Texture::getImage() const
{
    return m_image;
}

VkDescriptorImageInfo Texture::getInfo() const
{
    VkDescriptorImageInfo info{};
    info.sampler = m_sampler->getVkSampler();
    info.imageView = m_imageView;
    info.imageLayout = m_image->getVkImageLayout();

    return info;
}

}