#include "Material.h"
#include "Texture.h"
#include "utils/Constants.h"

namespace vke
{

Material::Material()
    : m_ambientColor{ 1.f },
    m_diffuseColor{ 1.f },
    m_specularColor{ 1.f },
    m_textureId{ RET_ID_NOT_FOUND },
    m_hasTexture{ false }
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

void Material::setTextureId(int id)
{
    m_textureId = id;
}

void Material::setTextureFile(std::string filename)
{
    m_textureFile = filename;
    m_hasTexture = true;
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

int Material::getTextureId() const
{
    return m_textureId;
}

std::string Material::getTextureFile() const
{
    return m_textureFile;
}

bool Material::hasTexture() const
{
    return m_hasTexture;
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