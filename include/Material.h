#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <string>

namespace vke
{

class Texture;

class Material
{
public:
    Material();
    ~Material();

    void setAmbientColor(glm::vec3 color);
    void setDiffuseColor(glm::vec3 color);
    void setSpecularColor(glm::vec3 color);
    void setOpacity(float opacity);
    void setTextureFile(std::string filename);
    void setTextureId(int id);

    glm::vec3 getAmbientColor() const;
    glm::vec3 getDiffuseColor() const;
    glm::vec3 getSpecularColor() const;
    float getOpacity() const;
    int getTextureId() const;
    std::string getTextureFile() const;

    bool hasTexture() const;

    void initTexture();
private:
    glm::vec3 m_ambientColor;
    glm::vec3 m_diffuseColor;
    glm::vec3 m_specularColor;

    std::string m_textureFile;
    int m_textureId;
    bool m_hasTexture;

    float m_opacity;
};

}