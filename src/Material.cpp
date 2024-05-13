/**
 * @file Material.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * 
 */

// vke
#include "Material.h"
#include "Texture.h"
#include "utils/Constants.h"

namespace vke
{

Material::Material()
    : m_ambientColor{ 1.f },
    m_diffuseColor{ 1.f },
    m_specularColor{ 1.f },
    m_opacity{ 1.f },
    m_textureId{ RET_ID_NOT_FOUND },
    m_hasTexture{ false },
    m_bumpTextureId{ RET_ID_NOT_FOUND },
    m_hasBumpTexture{ false }
{
    
}


Material::~Material()
{
}

void Material::setAmbientColor(const glm::vec3& color)
{
    m_ambientColor = color;
}

void Material::setDiffuseColor(const glm::vec3& color)
{
    m_diffuseColor = color;
}

void Material::setSpecularColor(const glm::vec3& color)
{
    m_specularColor = color;
}

void Material::setOpacity(const float& opacity)
{
    m_opacity = opacity;
}

void Material::setTextureId(const int& id)
{
    m_textureId = id;
}

void Material::setBumpTextureFile(const std::string& filename)
{
    m_bumpTextureFile = filename;
    m_hasBumpTexture = true;
}

void Material::setBumpTextureId(const int& id)
{
    m_bumpTextureId = id;
}

void Material::setHasBumpTexture(const bool& hasBumpMap)
{
    m_hasBumpTexture = hasBumpMap;
}

void Material::setTextureFile(const std::string& filename)
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

std::string Material::getBumpTextureFile() const
{
    return m_bumpTextureFile;
}

int Material::getBumpTextureId() const
{
    return m_bumpTextureId;
}

std::string Material::getTextureFile() const
{
    return m_textureFile;
}

bool Material::hasTexture() const
{
    return m_hasTexture;
}

bool Material::hasBumpTexture() const
{
    return m_hasBumpTexture;
}

bool Material::isTransparent() const
{
    return m_opacity != 1.f;
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