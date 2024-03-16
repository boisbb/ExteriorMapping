/**
 * @file RenderPass.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "RenderPass.h"

namespace vke
{

RenderPass::RenderPass(std::shared_ptr<Device> device, VkFormat colorFormat, VkFormat depthFormat,
    bool offscreen)
    : m_device(device), m_colorFormat(colorFormat), m_depthFormat(depthFormat), m_offscreen(offscreen)
{
    createRenderPass();
}

RenderPass::~RenderPass()
{
}

void RenderPass::destroyVkResources()
{
    vkDestroyRenderPass(m_device->getVkDevice(), m_renderPass, nullptr);
}

VkRenderPass RenderPass::getRenderPass() const
{
    return m_renderPass;
}

bool RenderPass::isOffscreen() const
{
    return m_offscreen;
}

void RenderPass::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_colorFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    if (m_offscreen)
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    else
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = m_depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    if (m_offscreen)
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    else
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE ;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    if (m_offscreen)
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    else
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::vector<VkSubpassDependency> colorDependencies;

    if (m_offscreen)
    {
        colorDependencies.resize(4);

        colorDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        colorDependencies[0].dstSubpass = 0;
        colorDependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        colorDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        colorDependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        colorDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        colorDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        colorDependencies[1].srcSubpass = 0;
        colorDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        colorDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        colorDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        colorDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        colorDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        colorDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        colorDependencies[2].srcSubpass = VK_SUBPASS_EXTERNAL;
        colorDependencies[2].dstSubpass = 0;
        colorDependencies[2].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        colorDependencies[2].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        colorDependencies[2].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        colorDependencies[2].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        colorDependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        colorDependencies[3].srcSubpass = 0;
        colorDependencies[3].dstSubpass = VK_SUBPASS_EXTERNAL;
        colorDependencies[3].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        colorDependencies[3].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        colorDependencies[3].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        colorDependencies[3].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        colorDependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }
    else
    {
        colorDependencies.resize(1);

        colorDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        colorDependencies[0].dstSubpass = 0;
        colorDependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        colorDependencies[0].srcAccessMask = 0;
        colorDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        colorDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    
    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(colorDependencies.size());
    renderPassInfo.pDependencies = colorDependencies.data();

    if (vkCreateRenderPass(m_device->getVkDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

}