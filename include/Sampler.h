#pragma once

#include "Device.h"

#include <vulkan/vulkan.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vke
{

class Sampler
{
public:
    Sampler(std::shared_ptr<Device> device, VkFilter filter, VkSamplerAddressMode wrap,
        VkSamplerMipmapMode mipMap);
    ~Sampler();

    VkSampler getVkSampler() const;

private:
    std::shared_ptr<Device> m_device;

    VkSampler m_sampler;
    VkFilter m_filter;
    VkSamplerAddressMode m_wrap;
    VkSamplerMipmapMode m_mipMap;
};

}