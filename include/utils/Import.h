#pragma once

#include "Model.h"
#include "Mesh.h"
#include "Structs.h"

#include <string>
#include <memory>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

namespace vke::utils
{

std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene, aiMatrix4x4 accTransform)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    aiVector3D UVW;
    aiVector3D n;

    for (uint32_t j = 0; j < mesh->mNumVertices; j++)
    {
        Vertex vertex;

        glm::vec3 position;
        position.x = mesh->mVertices[j].x;
        position.y = mesh->mVertices[j].y;
        position.z = mesh->mVertices[j].z; 

        vertex.pos = position;

        if (mesh->mTextureCoords[0])
        {
            glm::vec2 uv;
            uv.x = mesh->mTextureCoords[0][j].x;
            uv.y = mesh->mTextureCoords[0][j].y;

            // TODO: Add UVs to the Vertex struct
        }
        else
        {
            // TODO: Add DEFAULT UVs to the Vertex struct
        }

        if (mesh->HasNormals())
        {
            glm::vec3 normal;
            normal.x = mesh->mNormals[j].x;
            normal.y = mesh->mNormals[j].y;
            normal.z = mesh->mNormals[j].z;

            vertex.normal = normal;
        }

        // TODO: Check the index!!
        if (mesh->HasVertexColors(0))
        {
            glm::vec3 color;
            color.x = mesh->mColors[0][j].r;
            color.y = mesh->mColors[0][j].b;
            color.z = mesh->mColors[0][j].g;
            // TODO: alpha

            vertex.color = color;
        }
        else
        {
            vertex.color = { 0.5f, 0.5f, 0.5f };
        }

        vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // TODO: Materials
    //

    
    std::shared_ptr<Mesh> myMesh = std::make_shared<Mesh>(vertices, indices);

    std::cout << "Mesh info:" << std::endl;
    std::cout << vertices.size() << std::endl;
    std::cout << indices.size() << std::endl;

    return myMesh;
}

void processNode(std::shared_ptr<Model> model, aiNode* node, const aiScene* scene, aiMatrix4x4 accTransform)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];
        std::shared_ptr<Mesh> mesh = processMesh(aimesh, scene, accTransform);
        model->addMesh(mesh);
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(model, node->mChildren[i], scene, accTransform * node->mChildren[i]->mTransformation);
    }

    std::cout << "Num meshes(processNode): " << std::endl;
    std::cout << model->getMeshes().size() << std::endl;
}

std::shared_ptr<Model> importModel(std::string filename)
{
    Assimp::Importer import;
    const aiScene * scene = import.ReadFile(filename.c_str(), aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        throw std::runtime_error("Error loading model.");
    }

    std::shared_ptr<Model> model = std::make_shared<Model>();

    processNode(model, scene->mRootNode, scene, scene->mRootNode->mTransformation);

    std::cout << "Num meshes(importModel): " << std::endl;
    std::cout << model->getMeshes().size() << std::endl;

    return model;
}

}