#pragma once

#include <glm/glm.hpp>

namespace vke
{

class Material
{
public:
    Material();
    ~Material();

    void setAmbientColor(glm::vec3 color);
    void setDiffuseColor(glm::vec3 color);
    void setSpecularColor(glm::vec3 color);
    void setOpacity(float opacity);

    glm::vec3 getAmbientColor() const;
    glm::vec3 getDiffuseColor() const;
    glm::vec3 getSpecularColor() const;
    float getOpacity() const;
private:
    glm::vec3 m_ambientColor;
    glm::vec3 m_diffuseColor;
    glm::vec3 m_specularColor;

    float m_opacity;
};

}