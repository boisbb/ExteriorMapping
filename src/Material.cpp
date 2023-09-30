#include "Material.h"

namespace vke
{

Material::Material()
    : m_ambientColor{ 1.f },
    m_diffuseColor{ 1.f },
    m_specularColor{ 1.f }
{
    
}


Material::~Material()
{
}

void Material::setAmbientColor(glm::vec3 color)
{
    m_ambientColor = color;
}

void Material::setDiffuseColor(glm::vec3 color)
{
    m_diffuseColor = color;
}

void Material::setSpecularColor(glm::vec3 color)
{
    m_specularColor = color;
}

void Material::setOpacity(float opacity)
{
    m_opacity = opacity;
}

glm::vec3 Material::getAmbientColor() const
{
    return m_ambientColor;
}

glm::vec3 Material::getDiffuseColor() const
{
    return m_diffuseColor;
}

float Material::getOpacity() const
{
    return m_opacity;
}

glm::vec3 Material::getSpecularColor() const
{
    return m_specularColor;
}


}