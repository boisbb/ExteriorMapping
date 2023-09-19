#include "Camera.h"

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
    m_viewDirection(glm::normalize(center - eye))
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

void Camera::getCameraInfo(glm::vec3& eye, glm::vec3& center, glm::vec3& up, glm::vec3& viewDir, float& speed)
{
    eye = m_eye;
    center = m_center;
    up = m_up;
    viewDir = m_viewDirection;
    speed = m_moveSpeed;
}

void Camera::setCameraInfo(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up, const glm::vec3& viewDir, const float& speed)
{
    m_eye = eye;
    m_center = center;
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

    // Vulkan only.
    m_projection[1][1] *= -1.f;
}

}