#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>
#include <unordered_map>

#include "Texture.h"

namespace vke
{

class DescriptorSetLayout;
class DescriptorPool;
class Mesh;
class Device;

class Model
{
public:
    Model();
    ~Model();

    std::vector<std::shared_ptr<Mesh>> getMeshes() const;

    void addMesh(std::shared_ptr<Mesh> mesh);

    void afterImportInit(std::shared_ptr<Device> device,
        std::unordered_map<std::string, std::shared_ptr<Texture>>& textureMap);

    void draw(VkCommandBuffer commandBuffer);
private:
    std::vector<std::shared_ptr<Mesh>> m_meshes;
};

}