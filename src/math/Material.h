#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

struct Material {
    alignas(16) glm::vec3 albedo;
    alignas(16) glm::vec3 specular;
    alignas(16) glm::vec3 emission;
    alignas(4) float emissionStrength;
    alignas(4) float roughness;
    alignas(4) float specularHighlight;
    alignas(4) float specularExponent;
};

#endif // MATERIAL_H
