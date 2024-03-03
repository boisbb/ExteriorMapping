/**
 * @file Sampler.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "Sampler.h"

namespace vke
{

Sampler::Sampler(std::shared_ptr<Device> device, VkFilter filter, VkSamplerAddressMode wrap,
    VkSamplerMipmapMode mipMap)
    : m_device(device), m_filter(filter), m_wrap(wrap), m_mipMap(mipMap)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_device->getPhysicalDevice(), &properties);
    
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = m_filter;
    samplerInfo.minFilter = m_filter;
    samplerInfo.addressModeU = m_wrap;
    samplerInfo.addressModeV = m_wrap;
    samplerInfo.addressModeW = m_wrap;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = m_mipMap;
    samplerInfo.mipLodBias = 0.f;
    samplerInfo.minLod = 0.f;
    samplerInfo.maxLod = 0.f;

    if (vkCreateSampler(m_device->getVkDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed creating sampler");
    }
}

Sampler::~Sampler()
{
}

VkSampler Sampler::getVkSampler() const
{
    return m_sampler;
}

}