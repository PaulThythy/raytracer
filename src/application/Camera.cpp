#include "Camera.h"

Ray::Ray Camera::getRay(float u, float v) const {
    glm::vec3 w = glm::normalize(m_position - m_lookAt);
    glm::vec3 uVec = glm::normalize(glm::cross(m_up, w));
    glm::vec3 vVec = glm::cross(w, uVec);

    glm::vec3 origin = m_position;
    glm::vec3 direction = glm::normalize(uVec * u + vVec * v - w);

    return Ray::Ray(origin, direction);
}