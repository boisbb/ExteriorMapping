#pragma once

#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <memory>

namespace vke
{

struct Vertex;
class Buffer;
class Device;

class Mesh
{
public:
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
    ~Mesh();

    void afterImportInit(std::shared_ptr<Device> device);
    void draw(VkCommandBuffer commandBuffer);
private:
    void createVertexBuffer(std::shared_ptr<Device> device);
    void createIndexBuffer(std::shared_ptr<Device> device);

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    std::shared_ptr<Buffer> m_vertexBuffer;
    std::shared_ptr<Buffer> m_indexBuffer;
};

}