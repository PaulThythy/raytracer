#version 450

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
    vec3 position;
    vec3 lookAt;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 worldUp;
    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;
} cameraUBO;

struct Triangle {
    vec3 v0; 
    vec3 v1;
    vec3 v2;
};

layout(std140, set = 0, binding = 1) buffer Triangles {
    Triangle triangles[];
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

Ray getCameraRay(vec2 uv) {
    // Convertir les coordonnées UV de [0,1] à [-1,1]
    vec2 ndc = uv * 2.0 - 1.0;

    // Calcul du champ de vision en radians
    float fov = radians(cameraUBO.fov);

    // Calcul du plan image à la distance focale
    float imagePlaneHalfHeight = tan(fov / 2.0);
    float imagePlaneHalfWidth = imagePlaneHalfHeight * cameraUBO.aspectRatio;

    // Calcul de la direction du rayon dans l'espace caméra
    vec3 rayDirCameraSpace = normalize(
        ndc.x * imagePlaneHalfWidth * cameraUBO.right +
        ndc.y * imagePlaneHalfHeight * cameraUBO.up +
        cameraUBO.front
    );

    // Dans ce cas, l'espace monde et l'espace caméra sont les mêmes
    vec3 rayDirWorldSpace = normalize(rayDirCameraSpace);

    // Créer le rayon
    Ray ray;
    ray.origin = cameraUBO.position;
    ray.direction = rayDirWorldSpace;

    return ray;
}

bool rayIntersectsTriangle(Ray ray, Triangle tri, out float t) {
    const float EPSILON = 1e-6;
    vec3 edge1 = tri.v1 - tri.v0;
    vec3 edge2 = tri.v2 - tri.v0;
    vec3 h = cross(ray.direction, edge2);
    float a = dot(edge1, h);
    if (abs(a) < EPSILON)
        return false; // Le rayon est parallèle au triangle

    float f = 1.0 / a;
    vec3 s = ray.origin - tri.v0;
    float u = f * dot(s, h);
    if (u < 0.0 || u > 1.0)
        return false;

    vec3 q = cross(s, edge1);
    float v = f * dot(ray.direction, q);
    if (v < 0.0 || u + v > 1.0)
        return false;

    t = f * dot(edge2, q);
    if (t > EPSILON) {
        return true;
    } else {
        return false;
    }
}

struct Cylinder {
    vec3 baseCenter;
    vec3 axis;
    float radius;
    float height;
    vec4 color;
};

bool rayIntersectsCylinder(Ray ray, Cylinder cylinder, out float t) {
    // Calculer les variables nécessaires
    vec3 d = ray.direction;
    vec3 m = ray.origin - cylinder.baseCenter;
    vec3 n = cylinder.axis;

    // Calculer les coefficients de l'équation quadratique
    float mdn = dot(m, n);
    float ddn = dot(d, n);
    vec3 m_cross_n = m - mdn * n;
    vec3 d_cross_n = d - ddn * n;

    float a = dot(d_cross_n, d_cross_n);
    float b = 2.0 * dot(d_cross_n, m_cross_n);
    float c = dot(m_cross_n, m_cross_n) - cylinder.radius * cylinder.radius;

    // Résoudre l'équation quadratique a*t^2 + b*t + c = 0
    float discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) {
        return false; // Pas d'intersection
    }

    float sqrtDiscriminant = sqrt(discriminant);

    // Trouver les racines (t0 et t1)
    float t0 = (-b - sqrtDiscriminant) / (2.0 * a);
    float t1 = (-b + sqrtDiscriminant) / (2.0 * a);

    // Trier les racines
    if (t0 > t1) {
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    // Vérifier si les intersections sont dans les limites du cylindre
    float y0 = mdn + t0 * ddn;
    float y1 = mdn + t1 * ddn;

    if (y0 < 0.0) {
        if (y1 < 0.0) {
            return false; // Les deux intersections sont en dessous du cylindre
        } else {
            // Intersection avec le bas du cylindre
            t0 = t1;
            y0 = y1;
        }
    } else if (y0 > cylinder.height) {
        if (y1 > cylinder.height) {
            return false; // Les deux intersections sont au-dessus du cylindre
        } else {
            // Intersection avec le haut du cylindre
            t0 = t1;
            y0 = y1;
        }
    }

    // Intersection valide
    t = t0;
    return true;
}


Cylinder xAxisCylinder = Cylinder(
    vec3(0.0, 0.0, 0.0), 
    normalize(vec3(1.0, 0.0, 0.0)), 
    0.005, 
    1.0,  
    vec4(1.0, 0.0, 0.0, 1.0)
);

Cylinder yAxisCylinder = Cylinder(
    vec3(0.0, 0.0, 0.0), 
    normalize(vec3(0.0, 1.0, 0.0)), 
    0.005, 
    1.0,  
    vec4(0.0, 1.0, 0.0, 1.0) 
);

Cylinder zAxisCylinder = Cylinder(
    vec3(0.0, 0.0, 0.0),
    normalize(vec3(0.0, 0.0, 1.0)),
    0.005, 
    1.0,  
    vec4(0.0, 0.0, 1.0, 1.0)
);

void main() {
    Ray ray = getCameraRay(fragUV);

    bool hit = false;
    float closestT = 1e30;
    vec4 hitColor = vec4(0.0, 0.0, 0.0, 1.0);

    for (int i = 0; i < triangles.length(); ++i) {
        float t;
        if (rayIntersectsTriangle(ray, triangles[i], t)) {
            if (t < closestT) {
                closestT = t;
                hit = true;
                hitColor = vec4(1.0, 1.0, 0.0, 1.0);
            }
        }
    }

    float t;

    if (rayIntersectsCylinder(ray, xAxisCylinder, t)) {
        if (t < closestT && t > 0.0) {
            closestT = t;
            hit = true;
            hitColor = xAxisCylinder.color; 
        }
    }

    if (rayIntersectsCylinder(ray, yAxisCylinder, t)) {
        if (t < closestT && t > 0.0) {
            closestT = t;
            hit = true;
            hitColor = yAxisCylinder.color; 
        }
    }

    if (rayIntersectsCylinder(ray, zAxisCylinder, t)) {
        if (t < closestT && t > 0.0) {
            closestT = t;
            hit = true;
            hitColor = zAxisCylinder.color;
        }
    }

    if (hit) {
        outColor = hitColor; 
    } else {
        outColor = vec4(0.0, 0.0, 0.0, 1.0); 
    }
}