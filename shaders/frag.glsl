#version 450

#define SAMPLES 10
#define BOUNCES 20
#define PI 3.141592653589793238462643

layout(push_constant) uniform PushConstants {
    float uTime;
    //add uViewportSize
} pushConstants;

// From https://github.com/asc-community/MxEngine
float rand(vec2 co, float seed) {
    co *= fract(seed * 12.343);
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

layout(std140, set = 0, binding = 3) buffer Lights {
    Light lights[];
} lightBuffer;

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

    //TODO use uViewportSize
    float pixelScaleX = 2.0 / 1920.0;
    float pixelScaleY = 2.0 / 1080.0;

    // Generate random offsets for anti-aliasing within the pixel
    float randomOffsetX = (rand(vec2(float(sampleIndex), uv.x), sampleIndex) - 0.5) * pixelScaleX;
    float randomOffsetY = (rand(vec2(float(sampleIndex), uv.y), sampleIndex) - 0.5) * pixelScaleY;

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

vec3 sampleHemisphere(vec3 N, float seed) {
    float Xi1 = rand(fragUV + vec2(0.0, seed), seed);
    float Xi2 = rand(fragUV + vec2(seed, 0.0), seed);

    float theta = acos(sqrt(1.0 - Xi1));
    float phi = 2.0 * PI * Xi2;

    float xs = sin(theta) * cos(phi);
    float ys = cos(theta);
    float zs = sin(theta) * sin(phi);

    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangentX = normalize(cross(up, N));
    vec3 tangentY = cross(N, tangentX);

    vec3 direction = tangentX * xs + tangentY * zs + N * ys;
    return normalize(direction);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float denom = NdotV * (1.0 - k) + k;

    return NdotV / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 computeBRDF(Material material, vec3 N, vec3 V, vec3 L) {
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    vec3 F0 = mix(vec3(0.04), material.albedo, material.metallic);
    vec3 F = fresnelSchlick(VdotH, F0);

    float D = DistributionGGX(N, H, material.roughness);
    float G = GeometrySmith(N, V, L, material.roughness);

    vec3 numerator = D * F * G;
    float denominator = 4.0 * NdotV * NdotL + 0.001;
    vec3 specular = numerator / denominator;

    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - material.metallic;

    vec3 diffuse = kD * material.albedo / PI;

    return diffuse + specular;
}

bool traceRay(Ray ray, out HitRecord hitRecord) {
    float closestT = 1e20;
    bool hitSomething = false;

    for (int i = 0; i < sphereBuffer.spheres.length(); ++i) {
        float t;
        if (rayIntersectsSphere(ray, sphereBuffer.spheres[i], t)) {
            if (t < closestT) {
                closestT = t;
                hitSomething = true;
                hitRecord.position = ray.origin + t * ray.direction;
                hitRecord.normal = normalize(hitRecord.position - sphereBuffer.spheres[i].center);
                hitRecord.material = sphereBuffer.spheres[i].material;
            }
        }
    }

    for (int i = 0; i < trianglesBuffer.triangles.length(); ++i) {
        float t, u, v;
        if (rayIntersectsTriangle(ray, trianglesBuffer.triangles[i], t, u, v)) {
            if (t < closestT) {
                closestT = t;
                hitSomething = true;
                hitRecord.position = ray.origin + t * ray.direction;
                vec3 normal = normalize(
                    (1.0 - u - v) * trianglesBuffer.triangles[i].v0.normal +
                    u * trianglesBuffer.triangles[i].v1.normal +
                    v * trianglesBuffer.triangles[i].v2.normal
                );
                hitRecord.normal = normal;
                hitRecord.material = trianglesBuffer.triangles[i].material;
            }
        }
    }

    return hitSomething;
}

void main() {
    vec3 color = vec3(0.0);

    for (int sampleIndex = 0; sampleIndex < SAMPLES; ++sampleIndex) {
        Ray ray = getCameraRay(fragUV, sampleIndex);
        vec3 throughput = vec3(1.0);

        for (int bounce = 0; bounce < BOUNCES; ++bounce) {
            HitRecord hitRecord;
            if (traceRay(ray, hitRecord)) {
                color += throughput * hitRecord.material.emission * hitRecord.material.emissionStrength;

                vec3 N = normalize(hitRecord.normal);
                vec3 V = normalize(-ray.direction);

                int numLights = lightBuffer.lights.length();
                for (int i = 0; i < numLights; ++i) {
                    Light light = lightBuffer.lights[i];
                    vec3 L = normalize(light.position - hitRecord.position);
                    float distance = length(light.position - hitRecord.position);
                    float attenuation = 1.0 / (distance * distance);

                    Ray shadowRay;
                    shadowRay.origin = hitRecord.position + N * 0.001;
                    shadowRay.direction = L;

                    HitRecord shadowHit;
                    if (!traceRay(shadowRay, shadowHit) || length(shadowHit.position - hitRecord.position) > distance) {
                        vec3 BRDF = computeBRDF(hitRecord.material, N, V, L);

                        float NdotL = max(dot(N, L), 0.0);

                        vec3 radiance = light.color * light.intensity * attenuation;
                        color += throughput * BRDF * radiance * NdotL;
                    }
                }

                vec3 randomDir = sampleHemisphere(N, float(sampleIndex * BOUNCES + bounce));

                ray.origin = hitRecord.position + N * 0.001;
                ray.direction = randomDir;

                float NdotRandomDir = max(dot(N, randomDir), 0.0);
                float pdf = NdotRandomDir / PI;

                vec3 BRDF = computeBRDF(hitRecord.material, N, V, randomDir);

                throughput *= BRDF * NdotRandomDir / pdf;
            } else {
                break;
            }
        }
    }

    color /= float(SAMPLES);
    color = pow(color, vec3(1.0 / 2.2));
    outColor = vec4(color, 1.0);
}
