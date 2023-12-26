#include "Scene.h"
#include "Model.h"
#include "Mesh.h"
#include "Buffer.h"
#include "Camera.h"
#include "View.h"
#include "descriptors/SetLayout.h"
#include "descriptors/Set.h"
#include "descriptors/Pool.h"
#include "utils/Structs.h"
#include "utils/Constants.h"

namespace vke
{

Scene::Scene()
    : m_drawCount(0),
    m_sceneChanged(true),
    m_lightChanged(true)
{
}

Scene::~Scene()
{
}

void Scene::setModels(const std::shared_ptr<Device>& device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
    std::shared_ptr<DescriptorPool> descriptorPool, std::vector<std::shared_ptr<Model>> models,
    const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices)
{
    m_models = models;

    createVertexBuffer(device, vertices);
    createIndexBuffer(device, indices);
    createIndirectDrawBuffer(device);
}

void Scene::setLightChanged(bool lightChanged)
{
    m_lightChanged = lightChanged;
}

void Scene::setSceneChanged(bool sceneChanged)
{
    m_sceneChanged = sceneChanged;
}

std::vector<std::shared_ptr<Model>>& Scene::getModels()
{
    return m_models;
}

uint32_t Scene::getDrawCount() const
{
    return m_drawCount;
}

VkDrawIndexedIndirectCommand* Scene::getViewDrawData(std::shared_ptr<View> view, int currentFrame)
{
    return (VkDrawIndexedIndirectCommand*)m_indirectBuffersMap[view][currentFrame]->getMapped();
}

bool Scene::lightChanged() const
{
    return m_lightChanged;
}

bool Scene::sceneChanged() const
{
    return m_sceneChanged;
}

bool Scene::viewResourcesExist(std::shared_ptr<View> view)
{
    return m_computeDescriptorsMap.find(view) != m_computeDescriptorsMap.end();
}

void Scene::dispatch(std::shared_ptr<View> view, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
    uint32_t currentFrame)
{
    // VkDescriptorSet computeSet = m_computeDescriptorSets[currentFrame]->getDescriptorSet();
    // vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 1, 1, &computeSet, 0, nullptr);
    
    VkDescriptorSet computeSet = m_computeDescriptorsMap[view][currentFrame]->getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 1, 1, &computeSet, 0, nullptr);

    vkCmdDispatch(commandBuffer, 1, 1, 1);
}

void Scene::draw(std::shared_ptr<View> view, VkCommandBuffer commandBuffer,
    uint32_t currentFrame)
{
    VkBuffer vertexBuffers[] = { m_vertexBuffer->getVkBuffer() };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

    VkBuffer indirectDrawBuffer = m_indirectBuffersMap[view][currentFrame]->getVkBuffer();

    vkCmdDrawIndexedIndirect(commandBuffer, indirectDrawBuffer, 0, m_drawCount,
        sizeof(VkDrawIndexedIndirectCommand));
}

void Scene::createViewResources(std::shared_ptr<View> view, const std::shared_ptr<Device>& device,
        std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, std::shared_ptr<DescriptorPool> descriptorPool)
{
    if (m_computeDescriptorsMap.find(view) != m_computeDescriptorsMap.end())
    {
        return;
    }

    std::array<std::shared_ptr<Buffer>, MAX_FRAMES_IN_FLIGHT> drawBufferArray;
    std::array<std::shared_ptr<DescriptorSet>, MAX_FRAMES_IN_FLIGHT> descriptorArray;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        drawBufferArray[i] = std::make_shared<Buffer>(device, sizeof(VkDrawIndexedIndirectCommand) * (m_indirectDrawBuffer->getSize() + MAX_VIEWS),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        drawBufferArray[i]->map();

        drawBufferArray[i]->copyMapped(m_indirectDrawBuffer->getMapped(), m_indirectDrawBuffer->getSize());

        descriptorArray[i] = std::make_shared<DescriptorSet>(device, descriptorSetLayout, descriptorPool);

        std::vector<VkDescriptorBufferInfo> bufferInfos = {
            drawBufferArray[i]->getInfo()
        };

        std::vector<uint32_t> bufferBinding = {
            0
        };


        descriptorArray[i]->addBuffers(bufferBinding, bufferInfos);
    }

    m_computeDescriptorsMap[view] = descriptorArray;
    m_indirectBuffersMap[view] = drawBufferArray;
}

void Scene::addDebugViewCubesToDrawBuffer(const std::vector<std::shared_ptr<View>> &views)
{
    
}

void Scene::setLightPos(const glm::vec3& lightPos)
{
    m_lightPos = lightPos;
}

glm::vec3 Scene::getLightPos() const
{
    return m_lightPos;
}

void Scene::checkModelsVisible(std::shared_ptr<Camera> camera, int currentFrame)
{
    int id = 0;
    for (auto& model : m_models)
    {
        // model->checkMeshesVisible(camera, (VkDrawIndexedIndirectCommand*)m_indirectDrawBuffers[currentFrame]->getMapped());
    }
}

void Scene::createVertexBuffer(const std::shared_ptr<Device>& device,
    const std::vector<Vertex>& vertices)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    Buffer stagingBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.map();
    stagingBuffer.copyMapped((void*)vertices.data(), (size_t)bufferSize);
    stagingBuffer.unmap();

    m_vertexBuffer = std::make_shared<Buffer>(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    device->copyBuffer(stagingBuffer.getVkBuffer(), m_vertexBuffer->getVkBuffer(), m_vertexBuffer->getSize());
}

void Scene::createIndexBuffer(const std::shared_ptr<Device>& device,
    const std::vector<uint32_t> indices)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    Buffer stagingBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.map();
    stagingBuffer.copyMapped((void*)indices.data(), (size_t)bufferSize);
    stagingBuffer.unmap();
    
    m_indexBuffer = std::make_shared<Buffer>(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    device->copyBuffer(stagingBuffer.getVkBuffer(), m_indexBuffer->getVkBuffer(), m_indexBuffer->getSize());
}

void Scene::createIndirectDrawBuffer(const std::shared_ptr<Device>& device)
{
    std::vector<VkDrawIndexedIndirectCommand> commands;

    uint32_t instanceId = 0;

    for (auto& model : m_models)
        model->createIndirectDrawCommands(commands, instanceId);

    for (auto& model : m_models)
        model->createIndirectDrawCommandsTransparent(commands, instanceId);

    m_drawCount = instanceId;

    VkDeviceSize bufferSize = sizeof(VkDrawIndexedIndirectCommand) * commands.size();

    Buffer stagingBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.map();
    stagingBuffer.copyMapped((void*)commands.data(), (size_t)bufferSize);
    stagingBuffer.unmap();

    m_indirectDrawBuffer = std::make_shared<Buffer>(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_indirectDrawBuffer->map();

    device->copyBuffer(stagingBuffer.getVkBuffer(), m_indirectDrawBuffer->getVkBuffer(), m_indirectDrawBuffer->getSize());
}

}