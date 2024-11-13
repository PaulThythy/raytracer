#ifndef SCENE_H
#define SCENE_H

#include <iostream>
#include <vector>

#include "math/Material.h"
#include "math/Sphere.h"
#include "math/Triangle.h"

struct Scene {
    std::vector<Triangle> m_triangles;

    Scene();
};

#endif // SCENE_H
