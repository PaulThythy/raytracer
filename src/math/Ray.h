#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>

namespace Ray {

    struct Ray {
        glm::vec3 m_origin;
        glm::vec3 m_direction;

        Ray(const glm::vec3& o, const glm::vec3& d) : m_origin(o), m_direction(d) {}
    };
}

#endif