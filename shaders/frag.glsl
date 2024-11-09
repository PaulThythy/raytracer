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

layout(set = 0, binding = 1) buffer Triangles {
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

bool rayIntersectsTriangle(vec3 rayOrigin, vec3 rayDir, Triangle tri, out float t) {
    const float EPSILON = 1e-6;
    vec3 edge1 = tri.v1 - tri.v0;
    vec3 edge2 = tri.v2 - tri.v0;
    vec3 h = cross(rayDir, edge2);
    float a = dot(edge1, h);
    if (abs(a) < EPSILON)
        return false; // Le rayon est parallèle au triangle

    float f = 1.0 / a;
    vec3 s = rayOrigin - tri.v0;
    float u = f * dot(s, h);
    if (u < 0.0 || u > 1.0)
        return false;

    vec3 q = cross(s, edge1);
    float v = f * dot(rayDir, q);
    if (v < 0.0 || u + v > 1.0)
        return false;

    t = f * dot(edge2, q);
    if (t > EPSILON) {
        return true; // Intersection avec le triangle
    } else {
        return false; // Intersection derrière le rayon
    }
}

void main() {
    Ray ray = getCameraRay(fragUV);

    bool hit = false;
    float closestT = 1e30;

    // Parcourir tous les triangles
    for (int i = 0; i < triangles.length(); ++i) {
        float t;
        if (rayIntersectsTriangle(ray.origin, ray.direction, triangles[i], t)) {
            if (t < closestT) {
                closestT = t;
                hit = true;
            }
        }
    }

    if (hit) {
        outColor = vec4(1.0, 0.0, 0.0, 1.0); // Colorier en rouge si le rayon intersecte un triangle
    } else {
        outColor = vec4(0.0, 0.0, 0.0, 1.0); // Laisser le pixel noir sinon
    }
}