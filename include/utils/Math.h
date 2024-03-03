/**
 * @file Math.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "glm_include_unified.h"

namespace vke::utils
{

/**
 * @brief Get the scale value from glm matrix.
 * 
 * @param matrix 
 * @return glm::vec3 
 */
glm::vec3 getScaleFromMatrix(glm::mat4 matrix);

}