#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"
#include "RenderPass.h"
#include "Image.h"

namespace vke
{

class Sampler;

class Framebuffer
{
public:
    Framebuffer(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> renderPass, VkExtent2D resolution);
    Framebuffer(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> renderPass, VkExtent2D resolution, 
        VkImageView swapchainImageView);
    ~Framebuffer();

    VkFramebuffer getFramebuffer() const;
    std::shared_ptr<Sampler> getSampler() const;
    VkImageView getColorImageView() const;
    std::shared_ptr<Image> getColorImage() const; 
    VkDescriptorImageInfo getColorImageInfo();

private:
    void createImages();
    void createFramebuffer(std::shared_ptr<RenderPass> renderPass);

    VkFramebuffer m_framebuffer;

    std::shared_ptr<Device> m_device;

    std::shared_ptr<Image> m_colorImage;
    std::shared_ptr<Image> m_depthImage;

    VkImageView m_colorImageView;
    VkImageView m_depthImageView;

    std::shared_ptr<Sampler> m_sampler;

    VkExtent2D m_resolution;

    bool m_fromSwapchain = false;
};

}