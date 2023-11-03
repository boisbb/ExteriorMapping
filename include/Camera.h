#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vke
{

class Camera
{
public:
    Camera(glm::vec2 resolution, glm::vec3 eye,
        glm::vec3 center = glm::vec3(0.f, 0.f, 0.f),
        glm::vec3 up = glm::vec3(0.f, 1.f, 0.f),
        float nearPlane = 0.1f,
        float farPlane = 100.f,
        float fov = 45.f);
    ~Camera();

    glm::mat4 getView() const;
    glm::mat4 getProjection() const;
    void getCameraInfo(glm::vec3& eye, glm::vec3& up,
        glm::vec3& viewDir, float& speed);
    void getCameraRotateInfo(glm::vec2& resolution, float& sensitivity);
    glm::vec3 getEye() const;

    void setCameraInfo(const glm::vec3& eye,
        const glm::vec3& up, const glm::vec3& viewDir, const float& speed);

    void reconstructMatrices();
private:
    
    glm::vec2 m_resolution;
    glm::vec3 m_eye;
    glm::vec3 m_center;
    glm::vec3 m_up;
    glm::vec3 m_viewDirection;

    glm::mat4 m_view;
    glm::mat4 m_projection;

    float m_near;
    float m_far;
    float m_fov;

    float m_moveSpeed = 0.1f;
    float m_sensitivity = 100.f;
};

}