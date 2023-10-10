#include "utils/Import.h"

#include <filesystem>

namespace fs = std::filesystem;

namespace vke::utils
{
// Inspired by:
// https://stackoverflow.com/questions/29184311/how-to-rotate-a-skinned-models-bones-in-c-using-assimp
inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
{
    glm::mat4 to;

    to[0][0] = (float)from->a1; to[0][1] = (float)from->b1;  to[0][2] = (float)from->c1; to[0][3] = (float)from->d1;
    to[1][0] = (float)from->a2; to[1][1] = (float)from->b2;  to[1][2] = (float)from->c2; to[1][3] = (float)from->d2;
    to[2][0] = (float)from->a3; to[2][1] = (float)from->b3;  to[2][2] = (float)from->c3; to[2][3] = (float)from->d3;
    to[3][0] = (float)from->a4; to[3][1] = (float)from->b4;  to[3][2] = (float)from->c4; to[3][3] = (float)from->d4;

    return to;
}

std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene,
    const aiMatrix4x4& accTransform, std::string directory)
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

            vertex.uv.x = uv.x;
            vertex.uv.y = uv.y;
        }
        else
        {
            // TODO: Add DEFAULT UVs to the Vertex struct
            vertex.uv.x = 0.f;
            vertex.uv.y = 0.f;
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

    glm::mat4 modelMatrix = aiMatrix4x4ToGlm(&accTransform);

    std::shared_ptr<Mesh> myMesh = std::make_shared<Mesh>(vertices, indices);
    myMesh->setModelMatrix(modelMatrix);

    std::shared_ptr<Material> myMaterial = std::make_shared<Material>();

    // TODO: Materials
    if (mesh->mMaterialIndex >= 0)
    {
        std::cout << "Model has material!" << std::endl;

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiString textureFile;

        // TODO: Only caring about diffuse texture
        if (material->GetTextureCount(aiTextureType_AMBIENT))
        {
            material->GetTexture(aiTextureType_AMBIENT, 0, &textureFile);
            std::cout << "Ambient texture" << std::endl;
            std::cout << "Mtl name: " << material->GetName().C_Str() << std::endl;
            std::cout << "Texture file: " << textureFile.C_Str() << std::endl;
        }
        else if (material->GetTextureCount(aiTextureType_SPECULAR))
        {
            material->GetTexture(aiTextureType_SPECULAR, 0, &textureFile);
            std::cout << "Specular texture" << std::endl;
            std::cout << "Mtl name: " << material->GetName().C_Str() << std::endl;
            std::cout << "Texture file: " << textureFile.C_Str() << std::endl;
        }
        else if (material->GetTextureCount(aiTextureType_DIFFUSE))
        {
            material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFile);
            std::cout << "Diffuse texture" << std::endl;
            std::cout << "Mtl name: " << material->GetName().C_Str() << std::endl;
            std::cout << "Texture file: " << textureFile.C_Str() << std::endl;

            myMaterial->setTextureFile(directory + "/" + textureFile.C_Str());
        }
        else if (material->GetTextureCount(aiTextureType_UNKNOWN))
        {
            material->GetTexture(aiTextureType_UNKNOWN, 0, &textureFile);
            std::cout << "Unknown texture" << std::endl;
            std::cout << "Mtl name: " << material->GetName().C_Str() << std::endl;
            std::cout << "Texture file: " << textureFile.C_Str() << std::endl;
        }

        // Get colors
        aiColor3D ambient{};
        aiColor3D diffuse{};
        aiColor3D specular{};
        float opacity = 0.0;

        material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
        material->Get(AI_MATKEY_OPACITY, opacity);

        myMaterial->setAmbientColor({ ambient.r, ambient.g, ambient.b });
        myMaterial->setDiffuseColor({ diffuse.r, diffuse.g, diffuse.b });
        myMaterial->setSpecularColor({ specular.r, specular.g, specular.b });
        myMaterial->setOpacity(opacity);

        myMesh->setMaterial(myMaterial);
    }
    else
    {
        myMaterial->setAmbientColor({ 1.f, 1.f, 1.f });
        myMaterial->setDiffuseColor({ 1.f, 1.f, 1.f });
        myMaterial->setSpecularColor({ 1.f, 1.f, 1.f });
        myMaterial->setOpacity(1.f);
    }

    std::cout << "Mesh info:" << std::endl;
    std::cout << "vertices: " << vertices.size() << std::endl;
    std::cout << "indices: " << indices.size() << std::endl;
    std::cout << "has vertex colors: " << (bool)(mesh->HasVertexColors(0) == true) << std::endl;

    return myMesh;
}

void processNode(const std::shared_ptr<Model>& model, aiNode* node, const aiScene* scene,
    const aiMatrix4x4& accTransform, std::string directory)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];
        std::shared_ptr<Mesh> mesh = processMesh(aimesh, scene, accTransform, directory);
        model->addMesh(mesh);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(model, node->mChildren[i], scene, accTransform * node->mChildren[i]->mTransformation,
            directory);
    }
}

std::shared_ptr<Model> importModel(const std::string& filename)
{
    std::string directory;
    const size_t last_slash_idx = filename.rfind('/');
    if (std::string::npos != last_slash_idx)
    {
        directory = filename.substr(0, last_slash_idx);
    }

    fs::path dirPath = directory;

    Assimp::Importer import;
    const aiScene * scene = import.ReadFile(filename.c_str(), aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        throw std::runtime_error("Error loading model.");
    }

    std::shared_ptr<Model> model = std::make_shared<Model>();

    processNode(model, scene->mRootNode, scene, scene->mRootNode->mTransformation, fs::absolute(dirPath).c_str());

    return model;
}

}