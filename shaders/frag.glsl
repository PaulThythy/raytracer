#version 450

#define SAMPLES 1
#define BOUNCES 1
#define PI 3.141592653589793238462643

//from https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Material {
	vec3 albedo;
	vec3 specular;
	vec3 emission;
	float emissionStrength;
	float roughness;
	float specularHighlight;
	float specularExponent;
};

struct HitRecord {
	vec3 position;
	vec3 normal;
	Material material;
};

struct Triangle {
    vec3 v0; 
    vec3 v1;
    vec3 v2;
    Material material;
};

struct Cylinder {
    vec3 baseCenter;
    vec3 axis;
    float radius;
    float height;
    vec4 color;
};

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

Ray getCameraRay(vec2 uv, int sampleIndex) {
    // Convert UV coordinates from [0,1] to [-1,1].
    vec2 ndc = uv * 2.0 - 1.0;

    // Generate random offsets for anti-aliasing within the pixel
    float randomOffsetX = (rand(vec2(float(sampleIndex), uv.x)) - 0.5) / (float(SAMPLES) * 600);
    float randomOffsetY = (rand(vec2(float(sampleIndex), uv.y)) - 0.5) / (float(SAMPLES) * 600);

    // Apply random offsets to the UV coordinates
    ndc.x += randomOffsetX;
    ndc.y += randomOffsetY;

    // Field of view calculation in radians
    float fov = radians(cameraUBO.fov);

    // Calculation of the image plane at focal length
    float imagePlaneHalfHeight = tan(fov / 2.0);
    float imagePlaneHalfWidth = imagePlaneHalfHeight * cameraUBO.aspectRatio;

    // Calculation of beam direction in camera space
    vec3 rayDirCameraSpace = normalize(
        ndc.x * imagePlaneHalfWidth * cameraUBO.right +
        ndc.y * imagePlaneHalfHeight * cameraUBO.up +
        cameraUBO.front
    );

    // In this case, world space and camera space are the same
    vec3 rayDirWorldSpace = normalize(rayDirCameraSpace);

    // create the ray
    Ray ray;
    ray.origin = cameraUBO.position;
    ray.direction = rayDirWorldSpace;

    return ray;
}

Cylinder xAxisHelper = Cylinder(vec3(0.0, 0.0, 0.0), normalize(vec3(1.0, 0.0, 0.0)), 0.005, 1.0, vec4(1.0, 0.0, 0.0, 1.0));
Cylinder yAxisHelper = Cylinder(vec3(0.0, 0.0, 0.0), normalize(vec3(0.0, 1.0, 0.0)), 0.005, 1.0, vec4(0.0, 1.0, 0.0, 1.0));
Cylinder zAxisHelper = Cylinder(vec3(0.0, 0.0, 0.0), normalize(vec3(0.0, 0.0, 1.0)), 0.005, 1.0, vec4(0.0, 0.0, 1.0, 1.0));

bool rayIntersectsCylinder(Ray ray, Cylinder cylinder, out float t) {
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

layout(std140, set = 0, binding = 1) buffer Triangles {
    Triangle triangles[];
};

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

void main() {

    vec4 accumulatedColor = vec4(0.0);

    for (int i = 0; i < SAMPLES; ++i) {
        Ray ray = getCameraRay(fragUV, i);

        bool hit = false;
        float closestT = 1e30;
        vec4 hitColor = vec4(0.0, 0.0, 0.0, 1.0);

        for (int j = 0; j < triangles.length(); ++j) {
            float t;
            if (rayIntersectsTriangle(ray, triangles[j], t)) {
                if (t < closestT) {
                    closestT = t;
                    hit = true;
                    hitColor = vec4(triangles[j].material.albedo, 1.0);
                }
            }
        }

        float t;
        if (rayIntersectsCylinder(ray, xAxisHelper, t)) {
            if (t < closestT && t > 0.0) {
                closestT = t;
                hit = true;
                hitColor = xAxisHelper.color; 
            }
        }
        if (rayIntersectsCylinder(ray, yAxisHelper, t)) {
            if (t < closestT && t > 0.0) {
                closestT = t;
                hit = true;
                hitColor = yAxisHelper.color; 
            }
        }
        if (rayIntersectsCylinder(ray, zAxisHelper, t)) {
            if (t < closestT && t > 0.0) {
                closestT = t;
                hit = true;
                hitColor = zAxisHelper.color;
            }
        }

        accumulatedColor += (hit ? hitColor : vec4(0.0, 0.0, 0.0, 1.0));
    }

    // Average the color over the number of samples
    outColor = accumulatedColor / float(SAMPLES);
}