/**
 * @file Framebuffer.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "Framebuffer.h"
#include "Sampler.h"

namespace vke
{

Framebuffer::Framebuffer(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> renderPass, 
    VkExtent2D resolution)
    : m_device(device), m_resolution(resolution), m_colorImage(nullptr)
{
    createImages(renderPass);
    createFramebuffer(renderPass);
}

Framebuffer::Framebuffer(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> renderPass,
    VkExtent2D resolution, VkImageView swapchainImageView)
    : m_device(device), m_resolution(resolution), m_colorImage(nullptr), m_fromSwapchain(true),
    m_colorImageView(swapchainImageView)
{
    createImages(renderPass);
    createFramebuffer(renderPass);
}

Framebuffer::~Framebuffer()
{
    // destroyVkResources();
}

void Framebuffer::destroyVkResources()
{
    vkDestroyFramebuffer(m_device->getVkDevice(), m_framebuffer, nullptr);

    if (!m_fromSwapchain)
    {
        m_sampler->destroyVkResources();
        vkDestroyImageView(m_device->getVkDevice(), m_colorImageView, nullptr);
        m_colorImage->destroyVkResources();
        m_depthImage->destroyVkResources();
    }

    vkDestroyImageView(m_device->getVkDevice(), m_depthImageView, nullptr);
}

VkFramebuffer Framebuffer::getFramebuffer() const
{
    return m_framebuffer;
}

std::shared_ptr<Sampler> Framebuffer::getSampler() const
{
    return m_sampler;
}

VkImageView Framebuffer::getColorImageView() const
{
    return m_colorImageView;
}

std::shared_ptr<Image> Framebuffer::getColorImage() const
{
    return m_colorImage;
}

VkDescriptorImageInfo Framebuffer::getColorImageInfo()
{
    return VkDescriptorImageInfo{
        m_sampler->getVkSampler(),
        m_colorImageView,
        m_colorImage->getVkImageLayout()
    };
}

VkDescriptorImageInfo Framebuffer::getDepthImageInfo()
{
    return VkDescriptorImageInfo{
        m_sampler->getVkSampler(),
        m_depthImageView,
        m_depthImage->getVkImageLayout()
    };
}

VkExtent2D Framebuffer::getResolution() const
{
    return m_resolution;
}

void Framebuffer::createFramebuffer(std::shared_ptr<RenderPass> renderPass)
{
    std::array<VkImageView, 2> attachments = {
        m_colorImageView,
        m_depthImageView
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass->getRenderPass();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_resolution.width;
    framebufferInfo.height = m_resolution.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(m_device->getVkDevice(), &framebufferInfo, nullptr, &m_framebuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void Framebuffer::createImages(std::shared_ptr<RenderPass> renderPass)
{
    if (m_colorImage == nullptr && !m_fromSwapchain)
    {
        m_colorImage = std::make_shared<Image>(m_device, glm::vec2(m_resolution.width, m_resolution.height), VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
        m_colorImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        m_colorImage->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
        m_sampler = std::make_shared<Sampler>(m_device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_SAMPLER_MIPMAP_MODE_LINEAR);

        m_colorImageView = m_colorImage->createImageView();
    }

    if (renderPass->isOffscreen())
    {
        m_depthImage = std::make_shared<Image>(m_device, glm::vec2(m_resolution.width, m_resolution.height), m_device->getDepthFormat(),
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
        m_depthImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
        m_depthImage->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
    }
    else
    {
        m_depthImage = std::make_shared<Image>(m_device, glm::vec2(m_resolution.width, m_resolution.height), m_device->getDepthFormat(),
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    VkImageAspectFlags depthAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (m_device->getDepthFormat() >= VK_FORMAT_D16_UNORM_S8_UINT)
        depthAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    
    m_depthImageView = m_device->createImageView(m_depthImage->getVkImage(), m_device->getDepthFormat(), depthAspectFlags);
}

}