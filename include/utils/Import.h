/**
 * @file Import.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

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

#include "glm_include_unified.h"

namespace vke::utils
{

/**
 * @brief Assimp matrix to the glm matrix.
 * 
 * @param from Original assimp matrix.
 * @return glm::mat4 
 */
inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from);

/**
 * @brief Process the assimp mesh and parse it into vke::Mesh.
 * 
 * @param mesh Assimp mesh.
 * @param scene Assimp scene.
 * @param accTransform Current assimp transform of the model.
 * @param vertices All vertices parsed.
 * @param indices All indices parsed.
 * @param directory Directory for locating the assets.
 * @return std::shared_ptr<Mesh> 
 */
std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene,
    const aiMatrix4x4& accTransform, std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices, std::string directory);

/**
 * @brief Processes each individual node recursively.
 * 
 * @param model vke::Model dst model.
 * @param node Assimp node.
 * @param scene Assimp scene.
 * @param accTransform Current assimp transformation.
 * @param vertices All vertices parsed.
 * @param indices All indices parsed.
 * @param directory Directory for locating the assets.
 */
void processNode(const std::shared_ptr<Model>& model, aiNode* node, const aiScene* scene,
    const aiMatrix4x4& accTransform, std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices, std::string directory);

/**
 * @brief Import the obj model.
 * 
 * @param filename Path to the model.
 * @param vertices All vertices parsed.
 * @param indices All indices parsed.
 * @return std::shared_ptr<Model> 
 */
std::shared_ptr<Model> importModel(std::string filename, std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices);

}