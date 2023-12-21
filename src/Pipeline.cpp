#include "Pipeline.h"
#include "utils/FileHandling.h"

namespace vke
{

Pipeline::Pipeline(std::shared_ptr<Device> device)
    : m_device(device)
{
}

Pipeline::~Pipeline()
{
}

VkPipeline Pipeline::getPipeline() const
{
    return m_pipeline;
}

VkPipelineLayout Pipeline::getPipelineLayout() const
{
    return m_pipelineLayout;
}

VkShaderModule Pipeline::createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device->getVkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module");
    }

    return shaderModule;
}

}