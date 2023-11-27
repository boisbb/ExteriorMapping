#include "Camera.h"

#include <iostream>

namespace vke
{



Camera::Camera(glm::vec2 resolution, glm::vec3 eye, glm::vec3 center, glm::vec3 up,
    float nearPlane, float farPlane, float fov)
    : m_resolution(resolution),
    m_eye(eye),
    m_center(center),
    m_up(up),
    m_near(nearPlane),
    m_far(farPlane),
    m_fov(fov),
    m_viewDirection(glm::normalize(center - eye)),
    m_frustumPlanes(6)
{

}

Camera::~Camera()
{
}

glm::mat4 Camera::getView() const
{
    return m_view;
}

glm::mat4 Camera::getProjection() const
{
    return m_projection;
}

void Camera::getCameraInfo(glm::vec3& eye, glm::vec3& up, glm::vec3& viewDir, float& speed)
{
    eye = m_eye;
    up = m_up;
    viewDir = m_viewDirection;
    speed = m_moveSpeed;
}

void Camera::getCameraRotateInfo(glm::vec2& resolution, float& sensitivity)
{
    resolution = m_resolution;
    sensitivity = m_sensitivity;
}

glm::vec3 Camera::getEye() const
{
    return m_eye;
}

glm::vec4 Camera::getFrustum() const
{
    return m_frustum;
}

std::vector<glm::vec4> Camera::getFrustumPlanes() const
{
    return m_frustumPlanes;
}

glm::vec2 Camera::getNearFar() const
{
    return glm::vec2(m_near, m_far);
}

void Camera::setCameraInfo(const glm::vec3& eye,
    const glm::vec3& up, const glm::vec3& viewDir, const float& speed)
{
    m_eye = eye;
    m_up = up;
    m_viewDirection = viewDir;
    m_moveSpeed = speed;
}

void Camera::reconstructMatrices()
{
    m_view = glm::mat4(1.f);
    m_projection = glm::mat4(1.f);

    m_view = glm::lookAt(m_eye, m_eye + m_viewDirection, m_up);
    m_projection = glm::perspective(glm::radians(m_fov), m_resolution.x / m_resolution.y, m_near, m_far);

    float fov = glm::radians(m_fov);
    float aspect = m_resolution.x / m_resolution.y;

    float f = 1.f / tanf(fov / 2.f);
    glm::mat4 projection = glm::mat4(
        f / aspect, 0.f, 0.f, 0.f,
        0.f, f, 0.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
        0.f, 0.f, m_near, 0.f
    );

    // Vulkan only.
    m_projection[1][1] *= -1.f;

    buildFrustum();
}

void Camera::buildFrustum()
{
    glm::mat4 viewProjTransp = glm::transpose(m_projection * m_view);

    m_frustumPlanes[0] = viewProjTransp[3] + viewProjTransp[0];
    m_frustumPlanes[1] = viewProjTransp[3] - viewProjTransp[0];
    m_frustumPlanes[2] = viewProjTransp[3] - viewProjTransp[1];
    m_frustumPlanes[3] = viewProjTransp[3] + viewProjTransp[1];
    m_frustumPlanes[4] = viewProjTransp[3] + viewProjTransp[2];
    m_frustumPlanes[5] = viewProjTransp[3] - viewProjTransp[2];

    for (int i = 0; i < 6; ++i)
    {
        glm::vec4 p = m_frustumPlanes[i];
        m_frustumPlanes[i] = p / glm::length(glm::vec3(p));
    }
}

}