/**
 * @file Math.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "utils/Math.h"

namespace vke::utils
{
glm::vec3 getScaleFromMatrix(glm::mat4 matrix)
{
    glm::mat4 transformation;
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(transformation, scale, rotation, translation, skew, perspective);

    return scale;
}

}