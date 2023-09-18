#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace vke
{

class Mesh;
class Device;

class Model
{
public:
    Model();
    ~Model();

    std::vector<std::shared_ptr<Mesh>> getMeshes() const;

    void addMesh(std::shared_ptr<Mesh> mesh);

    void afterImportInit(std::shared_ptr<Device> device);

    void draw(VkCommandBuffer commandBuffer);
private:
    std::vector<std::shared_ptr<Mesh>> m_meshes;
};

}