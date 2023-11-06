#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
    void setBumpTextureFile(std::string filename);
    void setBumpTextureId(int id);
    void setHasBumpTexture(bool hasBumpMap);

    glm::vec3 getAmbientColor() const;
    glm::vec3 getDiffuseColor() const;
    glm::vec3 getSpecularColor() const;
    float getOpacity() const;
    std::string getTextureFile() const;
    int getTextureId() const;
    std::string getBumpTextureFile() const;
    int getBumpTextureId() const;

    bool hasTexture() const;
    bool hasBumpTexture() const;

    void initTexture();
private:
    glm::vec3 m_ambientColor;
    glm::vec3 m_diffuseColor;
    glm::vec3 m_specularColor;

    std::string m_textureFile;
    int m_textureId;
    bool m_hasTexture;

    std::string m_bumpTextureFile;
    int m_bumpTextureId;
    bool m_hasBumpTexture;

    float m_opacity;
};

}