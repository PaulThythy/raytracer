#ifndef PLANE_H
#define PLANE_H

#include "Ray.h"
#include "../HitableObject.h"

#include <glm/glm.hpp>

namespace Plane {

    struct Plane : public Hitable::HitableObject {
        glm::vec3 m_origin;
        glm::vec3 m_normal;

        Plane(const glm::vec3& origin, const glm::vec3& normal): m_origin(origin), m_normal(normal) {}

        bool intersect(const Ray::Ray& ray, float& t) const {
            double denom = glm::dot(m_normal, ray.m_direction);

            if (denom > 1e-6 || denom < -1e-6) {
                float Hd = glm::dot(m_origin - ray.m_origin, m_normal) / denom;
                if (Hd >= 0) {
                    t = Hd;
                    return true;
                }
            }
            return false;
        }
    };
}

#endif