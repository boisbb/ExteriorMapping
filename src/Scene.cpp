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
    m_lightChanged(true),
    m_renderDebugCameraGeometry(false),
    m_reinitializeDebugCameraGeometry(true),
    m_renderDebugViewsDrawCount(0)
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

    vkCmdDispatch(commandBuffer, 256, 1, 1);
}

void Scene::draw(std::shared_ptr<View> view, VkCommandBuffer commandBuffer,
    uint32_t currentFrame)
{
    VkBuffer vertexBuffers[] = { m_vertexBuffer->getVkBuffer() };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

    VkBuffer indirectDrawBuffer = m_indirectBuffersMap[view][currentFrame]->getVkBuffer();

    int totalDraws = m_drawCount;
    totalDraws += (m_renderDebugCameraGeometry) ? m_renderDebugViewsDrawCount : 0;

    vkCmdDrawIndexedIndirect(commandBuffer, indirectDrawBuffer, 0, totalDraws,
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

void Scene::hideModel(std::shared_ptr<Model> model)
{
    int drawStartOpaque = m_modelDrawRef[model][0];
    int drawStartTransparent = m_modelDrawRef[model][1];

    VkDrawIndexedIndirectCommand* commands = (VkDrawIndexedIndirectCommand*)m_indirectDrawBuffer->getMapped();

    for (int i = drawStartOpaque; i < drawStartOpaque + model->getMeshesCount(); i++)
    {
        commands[i].instanceCount = 0;
        commands[i].indexCount = 0;
    }

    for (int i = drawStartTransparent; i < drawStartTransparent + model->getTransparentMeshesCount(); i++)
    {
        commands[i].instanceCount = 0;
        commands[i].indexCount = 0;
    }

    for (auto& kv : m_indirectBuffersMap)
    {
        for (int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            std::shared_ptr<Buffer>& buffer = kv.second[j];

            buffer->copyMapped(m_indirectDrawBuffer->getMapped(), m_indirectDrawBuffer->getSize());

            VkDrawIndexedIndirectCommand* commands2 = (VkDrawIndexedIndirectCommand*)m_indirectBuffersMap[kv.first][j]->getMapped();

            std::vector<VkDrawIndexedIndirectCommand> t(commands2, commands2 + 1);
        }
    }
}

void Scene::removeView(std::shared_ptr<View> view)
{
    m_indirectBuffersMap.erase(view);
    m_computeDescriptorsMap.erase(view);
}

void Scene::addDebugCameraGeometry(std::vector<std::shared_ptr<View>> views)
{
    m_renderDebugCameraGeometry = true;
    if(!m_reinitializeDebugCameraGeometry)
        return;

    m_reinitializeDebugCameraGeometry = false;

    int startId = m_drawCount;

    VkDrawIndexedIndirectCommand* commands = (VkDrawIndexedIndirectCommand*)m_indirectDrawBuffer->getMapped();

    std::vector<VkDrawIndexedIndirectCommand> vectorCommands(commands, commands + m_drawCount);

    uint32_t instanceId = startId;

    std::cout << views.size() << std::endl;

    for (uint32_t i = 0; i < views.size(); i++)
    {
        views[i]->getDebugCameraModel()->createIndirectDrawCommands(vectorCommands, instanceId);
    }

    m_renderDebugViewsDrawCount = vectorCommands.size() - m_drawCount;

    std::cout << m_renderDebugViewsDrawCount << std::endl;

    m_indirectDrawBuffer->copyMapped((void*)vectorCommands.data(), sizeof(VkDrawIndexedIndirectCommand) * vectorCommands.size());

    for (auto& kv : m_indirectBuffersMap)
    {
        for (int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            kv.second[j]->copyMapped(m_indirectDrawBuffer->getMapped(), m_indirectDrawBuffer->getSize());
        }
    }

    for (int i = 0; i < views.size(); i++)
    {
        for (int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            VkDrawIndexedIndirectCommand* commands = (VkDrawIndexedIndirectCommand*)m_indirectBuffersMap[views[i]][j]->getMapped();

            int numOfMeshes = views[i]->getDebugCameraModel()->getMeshesCount();

            commands[startId + numOfMeshes * i].instanceCount = 1;
        }
    }
}

void Scene::setRenderDebugGeometryFlag(bool renderDebugCameraGeometryFlag)
{
    m_renderDebugCameraGeometry = renderDebugCameraGeometryFlag;
}

bool Scene::getRenderDebugGeometryFlag() const
{
    return m_renderDebugCameraGeometry;
}

void Scene::setReinitializeDebugCameraGeometryFlag(bool reinitializeDebugCameraGeometry)
{
    m_reinitializeDebugCameraGeometry = reinitializeDebugCameraGeometry;
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
    {
        m_modelDrawRef[model] = {
            static_cast<int>(commands.size()),
            -1
        };

        model->createIndirectDrawCommands(commands, instanceId);
    }

    for (auto& model : m_models)
    {
        m_modelDrawRef[model][1] = static_cast<int>(commands.size());
        model->createIndirectDrawCommandsTransparent(commands, instanceId);
    }

    m_drawCount = instanceId;

    VkDeviceSize bufferSize = sizeof(VkDrawIndexedIndirectCommand) * commands.size();

    Buffer stagingBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.map();
    stagingBuffer.copyMapped((void*)commands.data(), (size_t)bufferSize);
    stagingBuffer.unmap();

    m_indirectDrawBuffer = std::make_shared<Buffer>(device, bufferSize + MAX_VIEWS, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_indirectDrawBuffer->map();

    device->copyBuffer(stagingBuffer.getVkBuffer(), m_indirectDrawBuffer->getVkBuffer(), stagingBuffer.getSize());
}

}