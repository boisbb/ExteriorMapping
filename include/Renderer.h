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
class ViewGrid;
class DescriptorSetLayout;
class DescriptorPool;
class GraphicsPipeline;
class ComputePipeline;
class RenderPass;
class Framebuffer;
class Image;
class Sampler;

class Renderer
{
public:    
    Renderer(std::shared_ptr<Device> device, std::shared_ptr<Window> window, std::string vertexShaderFile,
        std::string fragmentShaderFile, std::string computeShaderFile, std::string quadVertexShaderFile,
        std::string quadFragmentShaderFile, std::string computeRaysEvalShaderFile);
    ~Renderer();

    void cullComputePass(const std::shared_ptr<Scene>& scene, const std::shared_ptr<ViewGrid>& viewGrid,
        bool novelViews = false);
    void rayEvalComputePass(const std::shared_ptr<ViewGrid>& novelViewGrid, 
        const std::shared_ptr<ViewGrid>& viewGrid);
    void renderPass(const std::shared_ptr<Scene>& scene, const std::shared_ptr<ViewGrid>& viewGrid, 
        const std::shared_ptr<ViewGrid>& viewMatrixGrid);
    void quadRenderPass(glm::vec2 windowResolution, bool depthOnly = false);
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
    void submitGraphics();
    void submitCompute();
    void presentFrame(const uint32_t& imageIndex, std::shared_ptr<Window> window, std::shared_ptr<View> view,
        bool& resizeViews);

    void changeQuadRenderPassSource(VkDescriptorImageInfo imageInfo);

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
    std::shared_ptr<RenderPass> getOffscreenRenderPass() const;
    std::shared_ptr<RenderPass> getQuadRenderPass() const;
    std::shared_ptr<Framebuffer> getOffscreenFramebuffer() const;
    std::shared_ptr<Framebuffer> getQuadFramebuffer(int imageIndex) const;
    std::shared_ptr<Framebuffer> getViewMatrixFramebuffer() const;
    VkDescriptorImageInfo getNovelImageInfo() const;
    SamplingType getNovelViewSamplingType() const;

    void setSceneChanged(int sceneChanged);
    void setLightChanged(int lightChanged);
    void setNovelViewSamplingType(SamplingType samplingType);

    int addTexture(std::shared_ptr<Texture> texture, std::string filename);
    int addBumpTexture(std::shared_ptr<Texture> texture, std::string filename);
    void addSecondaryWindow(std::shared_ptr<Window> window);

    void beginComputePass();
    void endComputePass();

    void beginCommandBuffer();
    void beginRenderPass(std::shared_ptr<RenderPass> renderPass, std::shared_ptr<Framebuffer> framebuffer, bool clear = true);
    void setNovelViewBarrier();
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

    void updateDescriptorData(const std::shared_ptr<Scene>& scene, const std::vector<std::shared_ptr<View>>& views, 
        const std::vector<std::shared_ptr<View>>& viewMatrix);
    void updateCullComputeDescriptorData(const std::shared_ptr<Scene>& scene);
    void updateRayEvalComputeDescriptorData(const std::vector<std::shared_ptr<View>>& novelViews,
        const std::vector<std::shared_ptr<View>>& views);
    void updateQuadDescriptorData(bool depthOnly);

    std::shared_ptr<Device> m_device;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<SwapChain> m_swapChain;
    std::shared_ptr<SwapChain> m_secondarySwapchain;
    std::shared_ptr<RenderPass> m_quadRenderPass;
    std::shared_ptr<RenderPass> m_offscreenRenderPass;
    std::shared_ptr<Framebuffer> m_mainFramebuffer;
    std::shared_ptr<Framebuffer> m_offscreenFramebuffer;
    std::shared_ptr<Framebuffer> m_viewMatrixFramebuffer;

    std::shared_ptr<GraphicsPipeline> m_offscreenPipeline;
    std::shared_ptr<ComputePipeline> m_cullPipeline;
    std::shared_ptr<ComputePipeline> m_raysEvalPipeline;
    std::shared_ptr<GraphicsPipeline> m_quadPipeline;
    // test
    std::shared_ptr<ComputePipeline> m_intersectsPipeline;
    std::shared_ptr<ComputePipeline> m_intervalsPipeline;

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
    std::vector<std::unique_ptr<Buffer>> m_creHitsCntssbo;
    std::vector<std::unique_ptr<Buffer>> m_quadubo;

    std::vector<std::shared_ptr<DescriptorSet>> m_generalDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_materialDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_computeDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_computeRayEvalDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_quadDescriptorSets;

    std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_viewSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_materialSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_computeSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_computeSceneSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_computeRayEvalSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_quadSetLayout;

    std::shared_ptr<DescriptorPool> m_descriptorPool;
    std::shared_ptr<DescriptorPool> m_viewPool;
    std::shared_ptr<DescriptorPool> m_materialPool;
    std::shared_ptr<DescriptorPool> m_computePool;
    std::shared_ptr<DescriptorPool> m_computeScenePool;
    std::shared_ptr<DescriptorPool> m_computeRayEvalPool;
    std::shared_ptr<DescriptorPool> m_quadPool;

    std::shared_ptr<Image> m_novelImage;
    std::shared_ptr<Sampler> m_novelImageSampler;
    VkImageView m_novelImageView;

    std::vector<int> bufferBindings;

    int m_currentFrame;
    int m_sceneFramesUpdated;
    int m_lightsFramesUpdated;
    SamplingType m_novelViewSamplingType;
};

}