#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>

struct Sphere {
    glm::vec3 m_center;
    float m_radius;

    Sphere(const glm::vec3& c, const float r) : m_center(c), m_radius(r) {}
};

#endif