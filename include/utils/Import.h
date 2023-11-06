#pragma once

#include "Model.h"
#include "Mesh.h"
#include "Structs.h"
#include "Material.h"

#include <string>
#include <memory>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vke::utils
{

inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from);
std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene,
    const aiMatrix4x4& accTransform, std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices, std::string directory);
void processNode(const std::shared_ptr<Model>& model, aiNode* node, const aiScene* scene,
    const aiMatrix4x4& accTransform, std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices, std::string directory);
std::shared_ptr<Model> importModel(const std::string& filename, std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices);

}