#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

struct Material {
    alignas(16) glm::vec3 m_albedo;
    alignas(16) glm::vec3 m_emission;
    alignas(4) float m_emissionStrength;
    alignas(4) float m_roughness;
    alignas(4) float m_metallic;
};

#endif // MATERIAL_H
