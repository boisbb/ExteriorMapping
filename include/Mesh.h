#pragma once

#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vke
{

struct Vertex;
class Buffer;
class Device;
class Material;

}

namespace vke
{

class Mesh
{
public:
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
    ~Mesh();

    void afterImportInit(std::shared_ptr<Device> device);
    void draw(VkCommandBuffer commandBuffer);

    void setModelMatrix(glm::mat4 matrix);
    void setMaterial(std::shared_ptr<Material> material);
private:
    void createVertexBuffer(std::shared_ptr<Device> device);
    void createIndexBuffer(std::shared_ptr<Device> device);

    glm::mat4 m_modelMatrix;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    std::shared_ptr<Buffer> m_vertexBuffer;
    std::shared_ptr<Buffer> m_indexBuffer;

    std::shared_ptr<Material> m_material;
};

}