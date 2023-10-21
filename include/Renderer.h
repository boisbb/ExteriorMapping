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
class Camera;
class DescriptorSetLayout;
class DescriptorPool;
class Pipeline;

class Renderer
{
public:
    Renderer(std::shared_ptr<Device> device, std::shared_ptr<Window> window, std::string vertexShaderFile,
        std::string fragmentShaderFile);
    ~Renderer();

    void initDescriptorResources();
    void renderFrame(std::vector<std::shared_ptr<Model>> models, std::shared_ptr<Camera> camera);

    std::shared_ptr<SwapChain> getSwapChain() const;
    VkCommandBuffer getCommandBuffer(int id) const;
    std::unordered_map<std::string, int>& getTextureMap();
    int getTextureId(std::string fileName);
    std::vector<std::shared_ptr<Texture>> getTextures() const;

    int addTexture(std::shared_ptr<Texture> texture, std::string filename);

    void beginRenderPass(int currentFrame, uint32_t imageIndex);
    void endRenderPass(int currentFrame);
private:
    void createCommandBuffers();
    void createDescriptors();
    void createPipeline(std::string vertexShaderFile, std::string fragmentShaderFile);

    void recordCommandBuffer(VkCommandBuffer commandBuffer, std::vector<std::shared_ptr<Model>> models,
        uint32_t imageIndex);
    void updateDescriptorBuffers(std::vector<std::shared_ptr<Model>> models, std::shared_ptr<Camera> camera);

    std::shared_ptr<Device> m_device;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<SwapChain> m_swapChain;
    std::shared_ptr<Pipeline> m_pipeline;

    std::unordered_map<std::string, int> m_textureMap;
    std::vector<std::shared_ptr<Texture>> m_textures;

    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<std::unique_ptr<Buffer>> ubos;
    std::vector<std::unique_ptr<Buffer>> vssbos;
    std::vector<std::unique_ptr<Buffer>> fssbos;

    std::vector<std::shared_ptr<DescriptorSet>> m_generalDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_textureDescriptorSets;

    std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_textureSetLayout;

    std::shared_ptr<DescriptorPool> m_descriptorPool;
    std::shared_ptr<DescriptorPool> m_texturePool;

    std::vector<int> bufferBindings;

    int m_currentFrame;
};

}