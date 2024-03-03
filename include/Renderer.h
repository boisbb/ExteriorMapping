/**
 * @file Renderer.h
 * @author Boris Burkalo (xburka00)
 * @brief Contains most of the rendering logic.
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

// Vulkan
#include <vulkan/vulkan.h>

// std
#include <unordered_map>

// vke
#include "Device.h"
#include "SwapChain.h"
#include "Texture.h"
#include "Buffer.h"

namespace vke
{

// Forward declarations.
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
    /**
     * @brief Construct a new Renderer object.
     * 
     * @param device Vulkan device to be used with the renderer.
     * @param window Main window for rendering.
     * @param params Contains paths to each individual shader needed.
     */
    Renderer(std::shared_ptr<Device> device, std::shared_ptr<Window> window, const RendererInitParams& params);
    ~Renderer();

    /**
     * @brief Initialize data for descriptors.
     * 
     */
    void initDescriptorResources();

    /**
     * @brief Performs the compute pass for Frustum culling.
     * 
     * @param scene Scene for evaluation.
     * @param viewGrid View grid through which the scene can be seen.
     * @param novelViews Flag for novel view rendering.
     */
    void cullComputePass(const std::shared_ptr<Scene>& scene, const std::shared_ptr<ViewGrid>& viewGrid,
        bool novelViews = false);

    /**
     * @brief Performs the compute pass for novel view generation.
     * 
     * @param novelViewGrid Grid which contains the novel view.
     * @param viewGrid Grid which contains the cameras to be used.
     * @param params Params for the compute pass.
     */
    void rayEvalComputePass(const std::shared_ptr<ViewGrid>& novelViewGrid, 
        const std::shared_ptr<ViewGrid>& viewGrid, const RayEvalParams& params);
    
    /**
     * @brief Renders the scene seen through view grid.
     * 
     * @param scene Scene to be rendered.
     * @param viewGrid Main view grid.
     * @param viewMatrixGrid View matrix.
     * @param updateData Flag whether to update corresponding data or not.
     */
    void renderPass(const std::shared_ptr<Scene>& scene, const std::shared_ptr<ViewGrid>& viewGrid, 
        const std::shared_ptr<ViewGrid>& viewMatrixGrid, bool updateData = true);

    /**
     * @brief Renders offscreen framebuffer to screen.
     * 
     * @param windowResolution Resolution of the window.
     * @param depthOnly Whether to render color or depth.
     * @param secondaryWindow Whether the target is secondary window.
     */
    void quadRenderPass(glm::vec2 windowResolution, bool depthOnly = false, bool secondaryWindow = false);
    
    /**
     * @brief Set the Viewport.
     * 
     * @param viewportStart Starting position of the viewport.
     * @param viewportResolution Resolution of the viewport.
     */
    void setViewport(const glm::vec2& viewportStart, const glm::vec2& viewportResolution);

    /**
     * @brief Set the Scissor.
     * 
     * @param viewportStart Starting position of the viewport.
     * @param viewportResolution Resolution of the viewport.
     */
    void setScissor(const glm::vec2& viewportStart, const glm::vec2& viewportResolution);

    /**
     * @brief Prepares necessary data for render pass.
     * 
     * @param scene Scene to be rendered.
     * @param window Window to render to.
     * @param secondarySwapchain Whether the secondary swapchain will be needed or not.
     */
    void prepareFrame(const std::shared_ptr<Scene>& scene, std::shared_ptr<Window> window,
        bool secondarySwapchain = false);

    /**
     * @brief Records the command buffer.
     * 
     * @param commandBuffer Command buffer. 
     * @param scene Scene.
     * @param view View through which the scene is seen.
     */
    void recordCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<Scene>& scene,
        const std::shared_ptr<View>& view);
    
    /**
     * @brief Records the compute command buffer.
     * 
     * @param commandBuffer Command buffer to use.
     * @param scene Scene.
     * @param view View through which the scene is seen.
     */
    void recordComputeCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<Scene>& scene,
        const std::shared_ptr<View>& view);

    /**
     * @brief Submit the frame for rendering.
     * 
     * @param secondarySwapchain Whether the secondary swapchain should be used.
     */
    void submitFrame(bool secondarySwapchain = false);

    /**
     * @brief Submit graphics without using swapchain.
     * 
     */
    void submitGraphics();

    /**
     * @brief Submit compute command buffer.
     * 
     */
    void submitCompute();

    /**
     * @brief Present the frame onto a surface.
     * 
     * @param window Window for presenting.
     * @param secondarySwapchain Whether to use secondary swapchain.
     */
    void presentFrame(std::shared_ptr<Window> window, bool secondarySwapchain = false);

    /**
     * @brief Change the source image for on screen render pass.
     * 
     * @param imageInfo Vulkan image info.
     */
    void changeQuadRenderPassSource(VkDescriptorImageInfo imageInfo);

    /**
     * @brief Copy offscreen frame buffer to the image for pixel testing.
     * 
     */
    void copyOffscreenFrameBufferToSupp();

    // Getters
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
    std::shared_ptr<Framebuffer> getQuadFramebuffer() const;
    std::shared_ptr<Framebuffer> getSecondaryQuadFramebuffer() const;
    std::shared_ptr<Framebuffer> getViewMatrixFramebuffer() const;
    VkDescriptorImageInfo getNovelImageInfo() const;
    VkDescriptorImageInfo getTestPixelImageInfo() const;
    SamplingType getNovelViewSamplingType() const;

    // Setters
    void setSceneChanged(int sceneChanged);
    void setLightChanged(int lightChanged);
    void setNovelViewSamplingType(SamplingType samplingType);

    /**
     * @brief Add texture to the texture map.
     * 
     * @param texture Texture to be added.
     * @param filename Its filename
     * @return int Texture id.
     */
    int addTexture(std::shared_ptr<Texture> texture, std::string filename);

    /**
     * @brief Add bump texture to the texture map.
     * 
     * @param texture Texture to be added.
     * @param filename Its filename
     * @return int Texture id.
     */
    int addBumpTexture(std::shared_ptr<Texture> texture, std::string filename);

    /**
     * @brief Add secondary window.
     * 
     * @param window Window to be added.
     */
    void addSecondaryWindow(std::shared_ptr<Window> window);

    /**
     * @brief Begin compute pass.
     * 
     */
    void beginComputePass();

    /**
     * @brief End compute pass.
     * 
     */
    void endComputePass();

    /**
     * @brief Begin command buffer.
     * 
     */
    void beginCommandBuffer();

    /**
     * @brief Begin render pass.
     * 
     * @param renderPass Render pass to be used.
     * @param framebuffer Framebuffer to be used.
     */
    void beginRenderPass(std::shared_ptr<RenderPass> renderPass, std::shared_ptr<Framebuffer> framebuffer);
    
    /**
     * @brief Set the image barrier for novel view image.
     * 
     */
    void setNovelViewBarrier();

    /**
     * @brief End render pass.
     * 
     */
    void endRenderPass();

    /**
     * @brief End command buffer.
     * 
     */
    void endCommandBuffer();
private:
    // Create methods.
    void createCommandBuffers();
    void createComputeCommandBuffers();
    void createDescriptors();
    void createRenderResources();
    void createPipeline(const RendererInitParams& params);

    /**
     * @brief Update the main desriptor data.
     * 
     * @param scene Scene.
     * @param views Main views.
     * @param viewMatrix View matrix views.
     */
    void updateDescriptorData(const std::shared_ptr<Scene>& scene, const std::vector<std::shared_ptr<View>>& views, 
        const std::vector<std::shared_ptr<View>>& viewMatrix);

    /**
     * @brief Update descriptor data for the cull compute pass.
     * 
     * @param scene Scene.
     */
    void updateCullComputeDescriptorData(const std::shared_ptr<Scene>& scene);

    /**
     * @brief Update data for the novel view generation.
     * 
     * @param novelViews 
     * @param views 
     * @param params 
     */
    void updateRayEvalComputeDescriptorData(const std::vector<std::shared_ptr<View>>& novelViews,
        const std::vector<std::shared_ptr<View>>& views, const RayEvalParams& params);
    
    /**
     * @brief Update data for the final on screen rendering.
     * 
     * @param depthOnly 
     */
    void updateQuadDescriptorData(bool depthOnly);

    std::shared_ptr<Device> m_device;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<SwapChain> m_swapChain;
    std::vector<uint32_t> m_swapChainImageIndices;
    std::shared_ptr<SwapChain> m_secondarySwapchain;
    std::vector<uint32_t> m_secondarySwapChainImageIndices;
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
    std::vector<std::unique_ptr<Buffer>> m_secondaryQuadubo;

    std::vector<std::shared_ptr<DescriptorSet>> m_generalDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_materialDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_computeDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_computeRayEvalDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_quadDescriptorSets;
    std::vector<std::shared_ptr<DescriptorSet>> m_secondaryQuadDescriptorSets;

    std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_viewSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_materialSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_computeSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_computeSceneSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_computeRayEvalSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_quadSetLayout;
    std::shared_ptr<DescriptorSetLayout> m_secondaryQuadSetLayout;

    std::shared_ptr<DescriptorPool> m_descriptorPool;
    std::shared_ptr<DescriptorPool> m_viewPool;
    std::shared_ptr<DescriptorPool> m_materialPool;
    std::shared_ptr<DescriptorPool> m_computePool;
    std::shared_ptr<DescriptorPool> m_computeScenePool;
    std::shared_ptr<DescriptorPool> m_computeRayEvalPool;
    std::shared_ptr<DescriptorPool> m_quadPool;
    std::shared_ptr<DescriptorPool> m_secondaryQuadPool;

    std::shared_ptr<Image> m_novelImage;
    std::shared_ptr<Sampler> m_novelImageSampler;
    VkImageView m_novelImageView;

    std::shared_ptr<Image> m_testPixelImage;
    std::shared_ptr<Sampler> m_testPixelSampler;
    VkImageView m_testPixelImageView;

    std::vector<int> bufferBindings;

    int m_currentFrame;
    int m_sceneFramesUpdated;
    int m_lightsFramesUpdated;
    SamplingType m_novelViewSamplingType;
};

}