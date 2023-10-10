#include "Material.h"
#include "Texture.h"

namespace vke
{

Material::Material()
    : m_ambientColor{ 1.f },
    m_diffuseColor{ 1.f },
    m_specularColor{ 1.f },
    m_texture{ nullptr }
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

void Material::setTexture(std::shared_ptr<Texture> texture)
{
    m_texture = texture;
}

void Material::setTextureFile(std::string filename)
{
    m_textureFile = filename;
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

std::shared_ptr<Texture> Material::getTexture() const
{
    return m_texture;
}

std::string Material::getTextureFile() const
{
    return m_textureFile;
}

bool Material::hasTexture() const
{
    return m_texture != nullptr;
}

void Material::initTexture()
{
    throw std::runtime_error("Not implemented");
}

glm::vec3 Material::getSpecularColor() const
{
    return m_specularColor;
}


}