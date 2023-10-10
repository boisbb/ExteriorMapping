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
    void setTexture(std::shared_ptr<Texture> texture);
    void setTextureFile(std::string filename);

    glm::vec3 getAmbientColor() const;
    glm::vec3 getDiffuseColor() const;
    glm::vec3 getSpecularColor() const;
    float getOpacity() const;
    std::shared_ptr<Texture> getTexture() const;
    std::string getTextureFile() const;

    bool hasTexture() const;

    void initTexture();
private:
    glm::vec3 m_ambientColor;
    glm::vec3 m_diffuseColor;
    glm::vec3 m_specularColor;

    std::string m_textureFile;
    std::shared_ptr<Texture> m_texture;

    float m_opacity;
};

}