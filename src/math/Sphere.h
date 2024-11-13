#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>
#include <vector>

#include "math/Triangle.h"
#include "math/Material.h"

struct Sphere {
    alignas(16) glm::vec3 m_center;
    alignas(4) float m_radius;
    Material m_material;

    Sphere(const glm::vec3& c, const float r, const Material& mat) : m_center(c), m_radius(r), m_material(mat) {}

    inline std::vector<Triangle> sphereGeometry(const unsigned int stacks, const unsigned int slices) {
        std::vector<Triangle> triangles;

        const float pi = 3.14159265358979323846f;
        const float twoPi = 2.0f * pi;
        float stackStep = pi / stacks;
        float sliceStep = twoPi / slices;

        std::vector<std::vector<glm::vec3>> vertices(stacks + 1, std::vector<glm::vec3>(slices + 1));

        for (unsigned int i = 0; i <= stacks; ++i) {
            float stackAngle = pi / 2.0f - i * stackStep; // from pi/2 to -pi/2
            float xy = m_radius * cosf(stackAngle);       // r * cos(theta)
            float z = m_radius * sinf(stackAngle);        // r * sin(theta)

            for (unsigned int j = 0; j <= slices; ++j) {
                float sliceAngle = j * sliceStep;         // from 0 to 2pi

                float x = xy * cosf(sliceAngle);           // r * cos(theta) * cos(phi)
                float y = xy * sinf(sliceAngle);           // r * cos(theta) * sin(phi)

                // Store the vertex position relative to the sphere's center
                vertices[i][j] = m_center + glm::vec3(x, y, z);
            }
        }

        // Generate triangles
        for (unsigned int i = 0; i < stacks; ++i) {
            for (unsigned int j = 0; j < slices; ++j) {
                // Define the four corners of the current quad
                glm::vec3 v1 = vertices[i][j];
                glm::vec3 v2 = vertices[i + 1][j];
                glm::vec3 v3 = vertices[i + 1][j + 1];
                glm::vec3 v4 = vertices[i][j + 1];

                // First triangle of the quad
                triangles.emplace_back(
                    Triangle(
                        Vertex3D(v1),
                        Vertex3D(v2),
                        Vertex3D(v3),
                        m_material
                    )
                );

                // Second triangle of the quad
                triangles.emplace_back(
                    Triangle(
                        Vertex3D(v1),
                        Vertex3D(v3),
                        Vertex3D(v4),
                        m_material
                    )
                );
            }
        }

        return triangles;
    }
};

#endif