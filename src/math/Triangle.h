#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Vertex.h"

#include <glm/glm.hpp>


struct Triangle {
    Vertex3D m_v0, m_v1, m_v2;

    Triangle(const Vertex3D& v0, const Vertex3D& v1, const Vertex3D& v2) : m_v0(v0), m_v1(v1), m_v2(v2) {}
};

#endif