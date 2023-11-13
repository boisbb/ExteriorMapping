#pragma once

#include <vulkan/vulkan.h>

#include <unordered_map>

#include "Device.h"
#include "SwapChain.h"
#include "Texture.h"
#include "Buffer.h"

namespace vke
{

class Model;
class Scene;
class Camera;
class DescriptorSetLayout;
class DescriptorPool;
class Pipeline;

class Renderer
{
public:    
    Renderer(std::shared_ptr<Device> device, std::shared_ptr<Window> window, std::string vertexShaderFile,
        std::string fragmentShaderFile, std::string computeShaderFile);
    ~Renderer();

    void initDescriptorResources();
    uint32_t prepareFrame(const std::shared_ptr<Scene>& scene, std::shared_ptr<Camera> camera);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<Scene>& scene,
        uint32_t imageIndex);
    void recordComputeCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<Scene>& scene);
    void presentFrame(const uint32_t& imageIndex);

    std::shared_ptr<SwapChain> getSwapChain() const;
    VkCommandBuffer getCommandBuffer(int id) const;
    std::unordered_map<std::string, int>& getTextureMap();
    int getTextureId(std::string fileName);
    std::vector<std::shared_ptr<Texture>> getTextures() const;
    int getBumpTextureId(std::string fileName);
    std::vector<std::shared_ptr<Texture>> getBumpTextures() const;
    int getCurrentFrame() const;
    VkCommandBuffer getCurrentCommandBuffer() const;
    std::shared_ptr<DescriptorSetLayout> getDescriptorSetLayout() const;
    std::shared_ptr<DescriptorPool> getDescriptorPool() const;
    std::shared_ptr<DescriptorSetLayout> getComputeDescriptorSetLayout() const;
    std::shared_ptr<DescriptorPool> getComputeDescriptorPool() const;

    int addTexture(std::shared_ptr<Texture> texture, std::string filename);
    int addBumpTexture(std::shared_ptr<Texture> texture, std::string filename);

    void beginRenderPass(int currentFrame, uint32_t imageIndex);
    void endRenderPass(int currentFrame);

    // testing
    void renderFrame(const std::shared_ptr<Scene>& scene, std::shared_ptr<Camera> camera);
private:
    void createCommandBuffers();
    void createComputeCommandBuffers();
    void createDescriptors();
    void createPipeline(std::string vertexShaderFile, std::string fragmentShaderFile,
        std::string computeShaderFile);

    void updateDescriptorData(const std::shared_ptr<Scene>& scene, std::shared_ptr<Camera> camera);

    std::shared_ptr<Device> m_device;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<SwapChain> m_swapChain;
    std::shared_ptr<Pipeline> m_pipeline;

    std::unordered_map<std::string, int> m_textureMap;
    std::vector<std::shared_ptr<Texture>> m_textures;

    std::unordered_map<std::string, int> m_bumpTextureMap;
    std::vector<std::shared_ptr<Texture>> m_bumpTextures;

    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkCommandBuffer> m_computeCommandBuffers;

    std::vector<std::unique_ptr<Buffer>> m_vubos;
    std::vector<std::unique_ptr<Buffer>> m_fubos;
    std::vector<std::unique_ptr<Buffer>> m_vssbos;
    std::vector<std::unique_ptr<Buffer>> m_fssbos;

    std::vector<std::shared_ptr<DescriptorSet>> m_generalDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_materialDescriptorSets;

    std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_materialSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_computeSetLayout;

    std::shared_ptr<DescriptorPool> m_descriptorPool;
    std::shared_ptr<DescriptorPool> m_materialPool;
    std::shared_ptr<DescriptorPool> m_computePool;

    std::vector<int> bufferBindings;

    int m_currentFrame;
};

}