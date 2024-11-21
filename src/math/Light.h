#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

struct Light {
    alignas(16) glm::vec3 m_position;
    alignas(16) glm::vec3 m_color;
    alignas(4) float m_intensity;
};

#endif