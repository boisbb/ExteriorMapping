#include "pipelines/ComputePipeline.h"
#include "utils/Constants.h"
#include "utils/FileHandling.h"

namespace vke
{

ComputePipeline::ComputePipeline(std::shared_ptr<Device> device, std::string compFile,
    std::vector<VkDescriptorSetLayout> computeSetLayouts)
    : Pipeline(device)
{
    create(compFile, computeSetLayouts);
}

ComputePipeline::~ComputePipeline()
{
}

void ComputePipeline::create(std::string compFile, std::vector<VkDescriptorSetLayout> computeSetLayouts)
{
    std::vector<char> compShaderCode = utils::readFile(COMPILED_SHADER_LOC + compFile);

    VkShaderModule compShaderModule = createShaderModule(compShaderCode);

    VkPipelineShaderStageCreateInfo compShaderStageInfo{};
    compShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    compShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    compShaderStageInfo.module = compShaderModule;
    compShaderStageInfo.pName = "main";

    VkPipelineLayoutCreateInfo computePipelineLayoutInfo{};
    computePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computePipelineLayoutInfo.setLayoutCount = computeSetLayouts.size();
    computePipelineLayoutInfo.pSetLayouts = computeSetLayouts.data();

    if (vkCreatePipelineLayout(m_device->getVkDevice(), &computePipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("fail while creating compute pipeline");

    VkComputePipelineCreateInfo computePipelineInfo{};
    computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineInfo.layout = m_pipelineLayout;
    computePipelineInfo.stage = compShaderStageInfo;

    if (vkCreateComputePipelines(m_device->getVkDevice(), VK_NULL_HANDLE, 1, &computePipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create compute pipeline");

    vkDestroyShaderModule(m_device->getVkDevice(), compShaderModule, nullptr);
}

void ComputePipeline::bind(VkCommandBuffer commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
}

}