/**
 * @file View.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-05-13
 * 
 * 
 */

#include "View.h"
#include "Buffer.h"
#include "Scene.h"
#include "Model.h"
#include "Mesh.h"
#include "Material.h"
#include "utils/Constants.h"

namespace vke
{

View::View(const glm::vec2& resolution, const glm::vec2& viewportStart, std::shared_ptr<Device> device, 
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, std::shared_ptr<DescriptorPool> descriptorPool)
    : m_resolution(resolution), m_viewportStart(viewportStart),
    m_camera(std::make_shared<Camera>(m_resolution, glm::vec3(2.f, 10.f, 2.f))),
    m_vubos(MAX_FRAMES_IN_FLIGHT), m_cubos(MAX_FRAMES_IN_FLIGHT), m_fubos(MAX_FRAMES_IN_FLIGHT),
    m_viewDescriptorSets(MAX_FRAMES_IN_FLIGHT), m_frustumCull(true), m_depthOnly(false)
{
    createDescriptorResources(device, descriptorSetLayout, descriptorPool);
}

View::~View()
{
}

void View::destroyVkResources(int currentFrame)
{
    if (currentFrame == -1)
    {
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_vubos[i]->destroyVkResources();
            m_cubos[i]->destroyVkResources();
            m_fubos[i]->destroyVkResources();
        }
    }
    else
    {
        m_vubos[currentFrame]->destroyVkResources();
        m_cubos[currentFrame]->destroyVkResources();
        m_fubos[currentFrame]->destroyVkResources();
    }
}

glm::vec2 View::getResolution() const
{
    return m_resolution;
}

glm::vec2 View::getViewportStart() const
{
    return m_viewportStart;
}

std::shared_ptr<Camera> View::getCamera() const
{
    return m_camera;
}
std::shared_ptr<DescriptorSet> View::getViewDescriptorSet(int currentFrame)
{
    return m_viewDescriptorSets[currentFrame];
}

bool View::getFrustumCull() const
{
    return m_frustumCull;
}

bool View::getDepthOnly() const
{
    return m_depthOnly;
}

glm::vec2 View::getNearFar() const
{
    return m_camera->getNearFar();
}

void View::setResolution(const glm::vec2 &resolution)
{
    m_resolution = resolution;

    m_camera->setCameraResolution(resolution);
}

void View::setViewportStart(const glm::vec2 &viewPortStart)
{
    m_viewportStart = viewPortStart;
}

void View::setCamera(std::shared_ptr<Camera> camera)
{
    m_camera = camera;
}

void View::setCameraEye(glm::vec3 eye)
{
    m_camera->setCameraEye(eye);
}

void View::setFrustumCull(bool frustumCull)
{
    m_frustumCull = frustumCull;
}

void View::setDepthOnly(bool depthOnly)
{
    m_depthOnly = depthOnly;
}

void View::updateDescriptorData(int currentFrame)
{
    ViewDataVertex vubo{};
    vubo.view = m_camera->getView();
    vubo.proj = m_camera->getProjection();

    m_vubos[currentFrame]->copyMapped(&vubo, sizeof(ViewDataVertex));

    ViewDataFragment fubo{};
    fubo.depthOnly = m_depthOnly;
    
    m_fubos[currentFrame]->copyMapped(&fubo, sizeof(ViewDataFragment));
}

void View::updateComputeDescriptorData(int currentFrame, const std::shared_ptr<Scene>& scene)
{
    ViewDataCompute cubo{};
    cubo.totalMeshes = scene->getDrawCount();
    cubo.frustumCull = m_frustumCull;

    std::vector<glm::vec4> frustumPlanes = m_camera->getFrustumPlanes();
    for (int i = 0; i < frustumPlanes.size(); i++)
    {
        cubo.frustumPlanes[i] = frustumPlanes[i];
    }

    m_cubos[currentFrame]->copyMapped(&cubo, sizeof(ViewDataCompute));
}

void View::updateDescriptorDataRenderDebugCube(std::vector<MeshShaderDataVertex>& vertexShaderData,
    std::vector<MeshShaderDataFragment>& fragmentShaderData)
{
    std::vector<std::shared_ptr<Mesh>> meshes = m_debugModel->getMeshes(); 

    for (int i = 0; i < meshes.size(); i++)
    {
        std::shared_ptr<Material> material = meshes[i]->getMaterial();

        glm::mat4 matrix = glm::inverse(m_camera->getView());
        matrix = glm::scale(matrix, glm::vec3(0.09f, 0.09f, 0.09f));

        vertexShaderData.push_back(MeshShaderDataVertex());
        vertexShaderData.back().model = matrix;

        fragmentShaderData.push_back(MeshShaderDataFragment());
        if (material->hasTexture())
            fragmentShaderData.back().multiple.y = material->getTextureId();
        else
            fragmentShaderData.back().multiple.y = RET_ID_NOT_FOUND;

        if (material->hasBumpTexture())
            fragmentShaderData.back().multiple.z = material->getBumpTextureId();
        else
            fragmentShaderData.back().multiple.z = RET_ID_NOT_FOUND;

        fragmentShaderData.back().diffuseColor = glm::vec4(material->getDiffuseColor(), 1.f);
        fragmentShaderData.back().multiple.x = material->getOpacity();
    }
}

void View::setDebugCameraGeometry(std::shared_ptr<Model> model)
{
    m_debugModel = model;
}

std::shared_ptr<Model> View::getDebugCameraModel() const
{
    return m_debugModel;
}

void View::createDescriptorResources(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout,
    std::shared_ptr<DescriptorPool> descriptorPool)
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_vubos[i] = std::make_unique<Buffer>(device, sizeof(ViewDataVertex),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_vubos[i]->map();

        m_cubos[i] = std::make_unique<Buffer>(device, sizeof(ViewDataCompute),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_cubos[i]->map();

        m_fubos[i] = std::make_unique<Buffer>(device, sizeof(ViewDataCompute),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_fubos[i]->map();
        
        m_viewDescriptorSets[i] = std::make_shared<DescriptorSet>(device, descriptorSetLayout, descriptorPool);

        std::vector<VkDescriptorBufferInfo> bufferInfos = {
            m_vubos[i]->getInfo(),
            m_cubos[i]->getInfo(),
            m_fubos[i]->getInfo(),
        };

        std::vector<uint32_t> bufferBinding = {
            0, 1, 2
        };

        m_viewDescriptorSets[i]->updateBuffers(bufferBinding, bufferInfos);
    }
}

}
