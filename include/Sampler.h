/**
 * @file Sampler.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * 
 */

#pragma once

#include "Device.h"

#include <vulkan/vulkan.h>

#include "glm_include_unified.h"

namespace vke
{

class Sampler
{
public:
    /**
     * @brief Construct a new Sampler object
     * 
     * @param device 
     * @param filter Filter for interpolation.
     * @param wrap Address mode for the texture.
     * @param mipMap Mip map mode.
     */
    Sampler(std::shared_ptr<Device> device, VkFilter filter, VkSamplerAddressMode wrap,
        VkSamplerMipmapMode mipMap);
    ~Sampler();

    void destroyVkResources();

    VkSampler getVkSampler() const;

private:
    std::shared_ptr<Device> m_device;

    VkSampler m_sampler;
    VkFilter m_filter;
    VkSamplerAddressMode m_wrap;
    VkSamplerMipmapMode m_mipMap;
};

}