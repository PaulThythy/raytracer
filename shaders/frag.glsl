#version 450

#define SAMPLES 1
#define BOUNCES 30
#define PI 3.141592653589793238462643

// From https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Material {
    vec3 albedo;
    vec3 emission;
    float emissionStrength;
    float roughness;
    float metallic;
};

struct HitRecord {
    vec3 position;
    vec3 normal;
    Material material;
};

struct Vertex {
    vec3 position;
    vec3 normal;
};

struct Triangle {
    Vertex v0;
    Vertex v1;
    Vertex v2;
    Material material;
};

struct Sphere {
    vec3 center;
    float radius;
    Material material;
};

layout(std140, set = 0, binding = 2) buffer Spheres {
    Sphere spheres[];
} sphereBuffer;

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

#define NUM_LIGHTS 1

// Create a list of lights
Light lights[NUM_LIGHTS] = Light[](
    Light(vec3(0.0, 0.0, 3.0), vec3(1.0, 1.0, 1.0), 1.0)
);

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(std140, set = 0, binding = 0) uniform UniformBufferObject {
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

layout(std140, set = 0, binding = 1) buffer Triangles {
    Triangle triangles[];
} trianglesBuffer;

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

    // Create the ray
    Ray ray;
    ray.origin = cameraUBO.position;
    ray.direction = rayDirWorldSpace;

    return ray;
}

bool rayIntersectsTriangle(Ray ray, Triangle tri, out float t, out float u, out float v) {
    const float EPSILON = 1e-6;
    vec3 edge1 = tri.v1.position - tri.v0.position;
    vec3 edge2 = tri.v2.position - tri.v0.position;
    vec3 h = cross(ray.direction, edge2);
    float a = dot(edge1, h);
    if (abs(a) < EPSILON)
        return false; // The ray is parallel to the triangle

    float f = 1.0 / a;
    vec3 s = ray.origin - tri.v0.position;
    u = f * dot(s, h);
    if (u < 0.0 || u > 1.0)
        return false;

    vec3 q = cross(s, edge1);
    v = f * dot(ray.direction, q);
    if (v < 0.0 || u + v > 1.0)
        return false;

    t = f * dot(edge2, q);
    if (t > EPSILON) {
        return true;
    } else {
        return false;
    }
}

bool rayIntersectsSphere(Ray ray, Sphere sphere, out float t) {
    vec3 oc = ray.origin - sphere.center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;

    float discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) {
        return false; // No intersection
    }

    float sqrtDiscriminant = sqrt(discriminant);
    float t0 = (-b - sqrtDiscriminant) / (2.0 * a);
    float t1 = (-b + sqrtDiscriminant) / (2.0 * a);

    // Find the nearest positive t
    if (t0 > 0.0) {
        t = t0;
        return true;
    }
    if (t1 > 0.0) {
        t = t1;
        return true;
    }
    return false; // Intersection behind the ray origin
}

vec3 randomHemisphereDirection(vec3 normal, float rand1, float rand2) {
    float phi = 2.0 * PI * rand1;
    float cosTheta = rand2;
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 tangent = normalize((abs(normal.x) > 0.999) ? cross(vec3(0.0, 1.0, 0.0), normal) : cross(vec3(1.0, 0.0, 0.0), normal));
    vec3 bitangent = cross(normal, tangent);

    return normalize(cos(phi) * sinTheta * tangent +
                     sin(phi) * sinTheta * bitangent +
                     cosTheta * normal);
}

vec3 calculateDirectLighting(HitRecord hitRecord) {
    vec3 Lo = vec3(0.0);
    vec3 N = normalize(hitRecord.normal);

    for (int i = 0; i < NUM_LIGHTS; ++i) {
        Light light = lights[i];
        vec3 L = normalize(light.position - hitRecord.position);
        vec3 H = normalize(L + normalize(-cameraUBO.position + hitRecord.position));
        float distance = length(light.position - hitRecord.position);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = light.color * light.intensity * attenuation;

        // Lambertian diffuse term
        float NdotL = max(dot(N, L), 0.0);
        vec3 diffuse = hitRecord.material.albedo * NdotL;

        // Specular term
        vec3 F0 = vec3(0.04); // Default reflectance at normal incidence
        F0 = mix(F0, hitRecord.material.albedo, hitRecord.material.metallic);
        float NDF = max(dot(N, H), 0.0);
        float G = max(dot(N, L), 0.0);
        vec3 F = F0 + (1.0 - F0) * pow(1.0 - dot(H, L), 5.0);

        vec3 specular = (NDF * G * F) / (4.0 * NdotL * max(dot(N, L), 0.0) + 0.001);

        Lo += (diffuse + specular) * radiance;
    }

    return Lo;
}

vec3 traceRay(Ray initialRay) {
    vec3 accumulatedColor = vec3(0.0);
    vec3 throughput = vec3(1.0);
    Ray ray = initialRay;

    for (int depth = 0; depth < BOUNCES; depth++) {
        float closestT = 1e30;
        HitRecord closestHitRecord;
        bool hit = false;

        // Check intersections with triangles
        for (int j = 0; j < trianglesBuffer.triangles.length(); ++j) {
            float t, u, v;
            if (rayIntersectsTriangle(ray, trianglesBuffer.triangles[j], t, u, v)) {
                if (t < closestT) {
                    closestT = t;
                    hit = true;
                    closestHitRecord.position = ray.origin + t * ray.direction;
                    closestHitRecord.normal = normalize(
                        (1.0 - u - v) * trianglesBuffer.triangles[j].v0.normal +
                        u * trianglesBuffer.triangles[j].v1.normal +
                        v * trianglesBuffer.triangles[j].v2.normal
                    );
                    closestHitRecord.material = trianglesBuffer.triangles[j].material;
                }
            }
        }

        // Check intersections with spheres
        for (int k = 0; k < sphereBuffer.spheres.length(); ++k) {
            float tSphere;
            if (rayIntersectsSphere(ray, sphereBuffer.spheres[k], tSphere)) {
                if (tSphere < closestT) {
                    closestT = tSphere;
                    hit = true;
                    closestHitRecord.position = ray.origin + tSphere * ray.direction;
                    closestHitRecord.normal = normalize(closestHitRecord.position - sphereBuffer.spheres[k].center);
                    closestHitRecord.material = sphereBuffer.spheres[k].material;
                }
            }
        }

        if (!hit) {
            accumulatedColor += throughput * vec3(0.0); // Background color
            break;
        }

        // Add emission for the current hit
        accumulatedColor += throughput * closestHitRecord.material.emission * closestHitRecord.material.emissionStrength;

        // Direct lighting contribution
        vec3 directLighting = calculateDirectLighting(closestHitRecord);
        accumulatedColor += throughput * directLighting;

        // Update throughput
        throughput *= closestHitRecord.material.albedo;

        // Generate a new direction
        float rand1 = rand(fragUV.xy + vec2(float(depth), 0.0));
        float rand2 = rand(fragUV.xy + vec2(0.0, float(depth)));
        vec3 randomDir = randomHemisphereDirection(closestHitRecord.normal, rand1, rand2);
        ray.origin = closestHitRecord.position + 0.001 * randomDir;
        ray.direction = randomDir;
    }

    return accumulatedColor;
}

void main() {
    vec3 accumulatedColor = vec3(0.0);

    for(int i = 0; i < SAMPLES; i++) {
        Ray ray = getCameraRay(fragUV, i);
        accumulatedColor += traceRay(ray);
    }

    accumulatedColor /= float(SAMPLES);
    accumulatedColor = pow(accumulatedColor, vec3(1.0 / 2.2)); //gamma correction
    outColor = vec4(accumulatedColor, 1.0);
}
