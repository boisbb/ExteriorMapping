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
    /**
     * @brief Construct a new Framebuffer object.
     * 
     * @param device Device.
     * @param renderPass Render pass in which the framebuffer will be used.
     * @param resolution Resolution of the framebuffer
     */
    Framebuffer(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> renderPass, VkExtent2D resolution);
    
    /**
     * @brief Construct a new Framebuffer object without creating images.
     * 
     * @param device 
     * @param renderPass 
     * @param resolution 
     * @param swapchainImageView Image view of the swapchain image.
     */
    Framebuffer(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> renderPass, VkExtent2D resolution, 
        VkImageView swapchainImageView);
    ~Framebuffer();

    void destroyVkResources();

    // Getters
    VkFramebuffer getFramebuffer() const;
    std::shared_ptr<Sampler> getSampler() const;
    VkImageView getColorImageView() const;
    std::shared_ptr<Image> getColorImage() const; 
    VkDescriptorImageInfo getColorImageInfo();
    VkDescriptorImageInfo getDepthImageInfo();
    VkExtent2D getResolution() const;

private:
    // Create methods
    void createImages(std::shared_ptr<RenderPass> renderPass);
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