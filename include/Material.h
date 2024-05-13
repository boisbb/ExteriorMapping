/**
 * @file Material.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * 
 */

#pragma once

// glm
#include "glm_include_unified.h"

// std
#include <memory>
#include <string>

namespace vke
{

class Texture;

class Material
{
public:
    /**
     * @brief Construct a new Material object
     * 
     */
    Material();
    ~Material();

    // Setters
    void setAmbientColor(const glm::vec3& color);
    void setDiffuseColor(const glm::vec3& color);
    void setSpecularColor(const glm::vec3& color);
    void setOpacity(const float& opacity);
    void setTextureFile(const std::string& filename);
    void setTextureId(const int& id);
    void setBumpTextureFile(const std::string& filename);
    void setBumpTextureId(const int& id);
    void setHasBumpTexture(const bool& hasBumpMap);

    // Getters
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
    bool isTransparent() const;

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