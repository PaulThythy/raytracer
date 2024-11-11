#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Vertex.h"
#include "Material.h"

#include <glm/glm.hpp>


struct Triangle {
    Vertex3D m_v0, m_v1, m_v2;
    Material m_material;

    Triangle(const Vertex3D& v0, const Vertex3D& v1, const Vertex3D& v2, const Material& mat) : m_v0(v0), m_v1(v1), m_v2(v2), m_material(mat) {}
};

#endif