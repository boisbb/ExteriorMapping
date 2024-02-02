#pragma once

#include "glm_include_unified.h"

#include <vector>

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
    glm::mat4 getViewInverse() const;
    glm::mat4 getProjection() const;
    glm::mat4 getProjectionInverse() const;
    void getCameraInfo(glm::vec3& eye, glm::vec3& up,
        glm::vec3& viewDir, float& speed);
    void getCameraRotateInfo(glm::vec2& resolution, float& sensitivity);
    glm::vec3 getEye() const;
    glm::vec4 getFrustum() const;
    std::vector<glm::vec4> getFrustumPlanes() const;
    glm::vec2 getNearFar() const;
    glm::vec2 getResolution() const;
    glm::vec3 getViewDir() const;
    float getSensitivity() const;

    void setCameraInfo(const glm::vec3& eye,
        const glm::vec3& up, const glm::vec3& viewDir, const float& speed);
    void setCameraResolution(const glm::vec2& resolution);
    void setCameraEye(glm::vec3 eye);
    void setViewDir(glm::vec3& viewDir);

    void reconstructMatrices();
private:

    void buildFrustum();
    
    glm::vec2 m_resolution;
    glm::vec3 m_eye;
    glm::vec3 m_center;
    glm::vec3 m_up;
    glm::vec3 m_viewDirection;
    glm::vec4 m_frustum;
    std::vector<glm::vec4> m_frustumPlanes;

    glm::mat4 m_view;
    glm::mat4 m_viewInverse;
    glm::mat4 m_projection;
    glm::mat4 m_projectionInverse;

    float m_near;
    float m_far;
    float m_fov;

    float m_moveSpeed = 0.1f;
    float m_sensitivity = 100.f;
};

}