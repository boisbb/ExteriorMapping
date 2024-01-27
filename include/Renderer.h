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
class View;
class DescriptorSetLayout;
class DescriptorPool;
class GraphicsPipeline;
class ComputePipeline;
class RenderPass;
class Framebuffer;

class Renderer
{
public:    
    Renderer(std::shared_ptr<Device> device, std::shared_ptr<Window> window, std::string vertexShaderFile,
        std::string fragmentShaderFile, std::string computeShaderFile, std::string quadVertexShaderFile,
        std::string quadFragmentShaderFile, std::string computeRaysEvalShaderFile);
    ~Renderer();

    void cullComputePass(const std::shared_ptr<Scene>& scene, const std::vector<std::shared_ptr<View>>& views);
    void rayEvalComputePass(const std::vector<std::shared_ptr<View>>& views);
    void renderPass(const std::shared_ptr<Scene>& scene, const std::vector<std::shared_ptr<View>> views);
    void quadRenderPass(glm::vec2 windowResolution);
    void setViewport(const glm::vec2& viewportStart, const glm::vec2& viewportResolution);
    void setScissor(const glm::vec2& viewportStart, const glm::vec2& viewportResolution);

    void initDescriptorResources();

    uint32_t prepareFrame(const std::shared_ptr<Scene>& scene, std::shared_ptr<View> view,
        std::shared_ptr<Window> window, bool& resizeViews);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<Scene>& scene,
        const std::shared_ptr<View>& view);
    void recordComputeCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<Scene>& scene,
        const std::shared_ptr<View>& view);
    void submitFrame();
    void submitCompute();
    void presentFrame(const uint32_t& imageIndex, std::shared_ptr<Window> window, std::shared_ptr<View> view,
        bool& resizeViews);

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
    std::shared_ptr<DescriptorSetLayout> getSceneComputeDescriptorSetLayout() const;
    std::shared_ptr<DescriptorPool> getSceneComputeDescriptorPool() const;
    std::shared_ptr<DescriptorSetLayout> getViewDescriptorSetLayout() const;
    std::shared_ptr<DescriptorPool> getViewDescriptorPool() const;
    VkRenderPass getOffscreenRenderPass() const;
    VkRenderPass getQuadRenderPass() const;
    VkFramebuffer getOffscreenFramebuffer() const;
    VkFramebuffer getQuadFramebuffer(int imageIndex);

    int addTexture(std::shared_ptr<Texture> texture, std::string filename);
    int addBumpTexture(std::shared_ptr<Texture> texture, std::string filename);

    void beginComputePass();
    void endComputePass();

    void beginCommandBuffer();
    void beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, glm::vec2 windowResolution,
        glm::uvec2 renderArea, bool clear = true);
    void endRenderPass();
    void endCommandBuffer();
private:
    void createCommandBuffers();
    void createComputeCommandBuffers();
    void createDescriptors();
    void createRenderResources();
    void createPipeline(std::string vertexShaderFile, std::string fragmentShaderFile,
        std::string computeShaderFile, std::string quadVertexShaderFile,
        std::string quadFragmentShaderFile, std::string computeRaysEvalShaderFile);

    void updateDescriptorData(const std::shared_ptr<Scene>& scene, const std::vector<std::shared_ptr<View>>& views);
    void updateCullComputeDescriptorData(const std::shared_ptr<Scene>& scene);
    void updateRayEvalComputeDescriptorData(const std::vector<std::shared_ptr<View>>& views);

    std::shared_ptr<Device> m_device;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<SwapChain> m_swapChain;
    std::shared_ptr<RenderPass> m_quadRenderPass;
    std::shared_ptr<RenderPass> m_offscreenRenderPass;
    std::shared_ptr<Framebuffer> m_mainFramebuffer;
    std::shared_ptr<Framebuffer> m_offscreenFramebuffer;
    std::shared_ptr<Framebuffer> m_cameraMatrixFramebuffer;

    std::shared_ptr<GraphicsPipeline> m_graphicsPipeline;
    std::shared_ptr<ComputePipeline> m_computePipeline;
    std::shared_ptr<ComputePipeline> m_computeRaysEvalPipeline;
    std::shared_ptr<GraphicsPipeline> m_quadPipeline;

    std::unordered_map<std::string, int> m_textureMap;
    std::vector<std::shared_ptr<Texture>> m_textures;

    std::unordered_map<std::string, int> m_bumpTextureMap;
    std::vector<std::shared_ptr<Texture>> m_bumpTextures;

    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkCommandBuffer> m_computeCommandBuffers;

    std::vector<std::unique_ptr<Buffer>> m_fubos;
    std::vector<std::unique_ptr<Buffer>> m_vssbos;
    std::vector<std::unique_ptr<Buffer>> m_fssbos;
    std::vector<std::unique_ptr<Buffer>> m_cssbos;
    std::vector<std::unique_ptr<Buffer>> m_creubo;
    std::vector<std::unique_ptr<Buffer>> m_cressbo;
    std::vector<std::unique_ptr<Buffer>> m_creDebugSsbo;
    std::vector<std::unique_ptr<Buffer>> m_creHitsssbo;

    std::vector<std::shared_ptr<DescriptorSet>> m_generalDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_materialDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_computeDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_computeRayEvalDescriptorSets;

    std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_viewSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_materialSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_computeSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_computeSceneSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_computeRayEvalSetLayout;

    std::shared_ptr<DescriptorPool> m_descriptorPool;
    std::shared_ptr<DescriptorPool> m_viewPool;
    std::shared_ptr<DescriptorPool> m_materialPool;
    std::shared_ptr<DescriptorPool> m_computePool;
    std::shared_ptr<DescriptorPool> m_computeScenePool;
    std::shared_ptr<DescriptorPool> m_computeRayEvalPool;

    std::vector<int> bufferBindings;

    int m_currentFrame;
    int m_sceneFramesUpdated;
    int m_lightsFramesUpdated;
};

}