#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Ray.h"
#include "HitableObject.h"

#include <glm/glm.hpp>

namespace Triangle {

    struct Triangle : public Hitable::HitableObject {
        glm::vec3 m_v0, m_v1, m_v2;

        Triangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) : m_v0(v0), m_v1(v1), m_v2(v2) {}

        bool intersect(const Ray::Ray& ray, float& t) const override {
            glm::vec3 v1v0 = m_v1 - m_v0;
            glm::vec3 v2v0 = m_v2 - m_v0;
            //plane's normal
            glm::vec3 N = glm::normalize(glm::cross(v1v0, v2v0));
            
            //step 1 : finding P

            //check if ray is parallel
            double NdotRayDirection = glm::dot(N, ray.m_direction);
            if (glm::abs(NdotRayDirection) < 1e-8) {
                return false;
            }

            //compute d parameter using equation 2
            double d = -glm::dot(N, m_v0);

            //compute t (equation 3)
            t = -(glm::dot(N, ray.m_origin) + d) / NdotRayDirection;

            // Check if the triangle is behind the ray
            if (t < 0) return false;

            // Compute the intersection point using equation 1
            glm::vec3 P = ray.m_origin + t * ray.m_direction;

            // Step 2: Inside-Outside Test
            glm::vec3 C;

            // Edge 0
            glm::vec3 edge0 = m_v1 - m_v0;
            glm::vec3 vp0 = P - m_v0;
            C = glm::cross(edge0, vp0);
            if (glm::dot(N, C) < 0) return false; // P is on the right side

            // Edge 1
            glm::vec3 edge1 = m_v2 - m_v1;
            glm::vec3 vp1 = P - m_v1;
            C = glm::cross(edge1, vp1);
            if (glm::dot(N, C) < 0) return false; // P is on the right side

            // Edge 2
            glm::vec3 edge2 = m_v0 - m_v2;
            glm::vec3 vp2 = P - m_v2;
            C = glm::cross(edge2, vp2);
            if (glm::dot(N, C) < 0) return false; // P is on the right side

            return true; // This ray hits the triangle
        }
    };
}

#endif