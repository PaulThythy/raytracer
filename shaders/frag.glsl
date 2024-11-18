#version 450

#define SAMPLES 1
#define BOUNCES 10
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

struct Cylinder {
    vec3 baseCenter;
    vec3 axis;
    float radius;
    float height;
    vec4 color;
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
    Light(vec3(0.0, 0.0, 3.0), vec3(1.0, 1.0, 1.0), 0.8)
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

Cylinder xAxisHelper = Cylinder(vec3(0.0, 0.0, 0.0), normalize(vec3(1.0, 0.0, 0.0)), 0.005, 1.0, vec4(1.0, 0.0, 0.0, 1.0));
Cylinder yAxisHelper = Cylinder(vec3(0.0, 0.0, 0.0), normalize(vec3(0.0, 1.0, 0.0)), 0.005, 1.0, vec4(0.0, 1.0, 0.0, 1.0));
Cylinder zAxisHelper = Cylinder(vec3(0.0, 0.0, 0.0), normalize(vec3(0.0, 0.0, 1.0)), 0.005, 1.0, vec4(0.0, 0.0, 1.0, 1.0));

bool rayIntersectsCylinder(Ray ray, Cylinder cylinder, out float t) {
    vec3 d = ray.direction;
    vec3 m = ray.origin - cylinder.baseCenter;
    vec3 n = cylinder.axis;

    // Calculate the coefficients of the quadratic equation
    float mdn = dot(m, n);
    float ddn = dot(d, n);
    vec3 m_cross_n = m - mdn * n;
    vec3 d_cross_n = d - ddn * n;

    float a = dot(d_cross_n, d_cross_n);
    float b = 2.0 * dot(d_cross_n, m_cross_n);
    float c = dot(m_cross_n, m_cross_n) - cylinder.radius * cylinder.radius;

    // Solve the quadratic equation a*t^2 + b*t + c = 0
    float discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) {
        return false; // No intersection
    }

    float sqrtDiscriminant = sqrt(discriminant);

    // Find the roots (t0 and t1)
    float t0 = (-b - sqrtDiscriminant) / (2.0 * a);
    float t1 = (-b + sqrtDiscriminant) / (2.0 * a);

    // Sort the roots
    if (t0 > t1) {
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    // Check if the intersections are within the cylinder's bounds
    float y0 = mdn + t0 * ddn;
    float y1 = mdn + t1 * ddn;

    if (y0 < 0.0) {
        if (y1 < 0.0) {
            return false; // Both intersections are below the cylinder
        } else {
            // Intersection with the bottom cap of the cylinder
            t0 = t1;
            y0 = y1;
        }
    } else if (y0 > cylinder.height) {
        if (y1 > cylinder.height) {
            return false; // Both intersections are above the cylinder
        } else {
            // Intersection with the top cap of the cylinder
            t0 = t1;
            y0 = y1;
        }
    }

    // Valid intersection
    t = t0;
    return true;
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

vec3 calculateLighting(HitRecord hitRecord, Ray ray) {
    vec3 color = vec3(0.0);

    for (int i = 0; i < NUM_LIGHTS; ++i) {
        Light light = lights[i];

        // Calculate light direction
        vec3 lightDir = normalize(light.position - hitRecord.position);

        // Ambient Component
        vec3 ambient = 0.1 * hitRecord.material.albedo;

        // Diffuse Component
        float diff = max(dot(hitRecord.normal, lightDir), 0.0);
        vec3 diffuse = diff * hitRecord.material.albedo * light.color * light.intensity;

        // Specular Component
        vec3 viewDir = normalize(-ray.direction);
        vec3 reflectDir = reflect(-lightDir, hitRecord.normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), hitRecord.material.specularExponent);
        vec3 specular = spec * hitRecord.material.specular * light.color * light.intensity;

        color += ambient + diffuse + specular;
    }

    // Add emission if any
    color += hitRecord.material.emission * hitRecord.material.emissionStrength;

    return color;
}

void main() {
    vec4 accumulatedColor = vec4(0.0);

    for (int i = 0; i < SAMPLES; i++) {
        Ray ray = getCameraRay(fragUV, i);

        bool hit = false;
        float closestT = 1e30;
        HitRecord closestHitRecord;

        // Iterate over triangles
        for (int j = 0; j < trianglesBuffer.triangles.length(); ++j) {
            float t;
            float u, v;
            if (rayIntersectsTriangle(ray, trianglesBuffer.triangles[j], t, u, v)) {
                if (t < closestT && t > 0.0) {
                    closestT = t;
                    hit = true;
                    closestHitRecord.position = ray.origin + t * ray.direction;

                    // Interpolate normals using barycentric coordinates
                    closestHitRecord.normal = normalize(
                        (1.0 - u - v) * trianglesBuffer.triangles[j].v0.normal +
                        u * trianglesBuffer.triangles[j].v1.normal +
                        v * trianglesBuffer.triangles[j].v2.normal
                    );

                    closestHitRecord.material = trianglesBuffer.triangles[j].material;
                }
            }
        }

        // Iterate over spheres
        for (int k = 0; k < sphereBuffer.spheres.length(); ++k) {
            float tSphere;
            if (rayIntersectsSphere(ray, sphereBuffer.spheres[k], tSphere)) {
                if (tSphere < closestT && tSphere > 0.0) {
                    closestT = tSphere;
                    hit = true;
                    closestHitRecord.position = ray.origin + tSphere * ray.direction;

                    // Calculate normal at the intersection point
                    closestHitRecord.normal = normalize(closestHitRecord.position - sphereBuffer.spheres[k].center);

                    // Assign the sphere's material
                    closestHitRecord.material = sphereBuffer.spheres[k].material;
                }
            }
        }

        // Iterate over cylinders
        float t_cyl;
        if (rayIntersectsCylinder(ray, xAxisHelper, t_cyl)) {
            if (t_cyl < closestT && t_cyl > 0.0) {
                closestT = t_cyl;
                hit = true;
                closestHitRecord.position = ray.origin + t_cyl * ray.direction;
                closestHitRecord.normal = normalize(ray.origin + t_cyl * ray.direction - xAxisHelper.baseCenter);
                closestHitRecord.material.albedo = xAxisHelper.color.rgb;
                closestHitRecord.material.specular = vec3(0.0);
                closestHitRecord.material.emission = vec3(0.0);
                closestHitRecord.material.emissionStrength = 0.0;
                closestHitRecord.material.specularExponent = 32.0;
            }
        }
        if (rayIntersectsCylinder(ray, yAxisHelper, t_cyl)) {
            if (t_cyl < closestT && t_cyl > 0.0) {
                closestT = t_cyl;
                hit = true;
                closestHitRecord.position = ray.origin + t_cyl * ray.direction;
                closestHitRecord.normal = normalize(ray.origin + t_cyl * ray.direction - yAxisHelper.baseCenter);
                closestHitRecord.material.albedo = yAxisHelper.color.rgb;
                closestHitRecord.material.specular = vec3(0.0);
                closestHitRecord.material.emission = vec3(0.0);
                closestHitRecord.material.emissionStrength = 0.0;
                closestHitRecord.material.specularExponent = 32.0;
            }
        }
        if (rayIntersectsCylinder(ray, zAxisHelper, t_cyl)) {
            if (t_cyl < closestT && t_cyl > 0.0) {
                closestT = t_cyl;
                hit = true;
                closestHitRecord.position = ray.origin + t_cyl * ray.direction;
                closestHitRecord.normal = normalize(ray.origin + t_cyl * ray.direction - zAxisHelper.baseCenter);
                closestHitRecord.material.albedo = zAxisHelper.color.rgb;
                closestHitRecord.material.specular = vec3(0.0);
                closestHitRecord.material.emission = vec3(0.0);
                closestHitRecord.material.emissionStrength = 0.0;
                closestHitRecord.material.specularExponent = 32.0;
            }
        }

        vec3 color;
        if (hit) {
            color = calculateLighting(closestHitRecord, ray);
        } else {
            // Background color
            color = vec3(0.0);
        }

        accumulatedColor += vec4(color, 1.0);
    }

    // Average the color over the number of samples
    outColor = accumulatedColor / float(SAMPLES);
}
