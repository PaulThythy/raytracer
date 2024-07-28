#version 450

layout(location=1) in vec2 fragUV;

layout(location=0) out vec4 outColor;

//sphere infos
struct Sphere {
    vec3 center;
    float radius;
    vec4 color;
};

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;


const Sphere spheres[] = {
    Sphere(vec3(0.0, 0.0, -60.0), 1.0, vec4(1.0, 0.0, 0.0, 1.0)),
    Sphere(vec3(0.0, 4.0, -60.0), 0.3, vec4(0.0, 0.0, 1.0, 1.0))
};

bool sphereHit(vec3 rayOrigin, vec3 rayDirection, Sphere sphere, out float t) {
    vec3 oc = rayOrigin - sphere.center;
    float a = dot(rayDirection, rayDirection);
    float b = 2.0 * dot(oc, rayDirection);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;

    float discriminant = b * b - 4.0 * a * c;

    if (discriminant > 0.0) {
        float sqrtD = sqrt(discriminant);
        float t0 = (-b - sqrtD) / (2.0 * a);
        float t1 = (-b + sqrtD) / (2.0 * a);

        if (t0 > t1) {
            float temp = t0;
            t0 = t1;
            t1 = temp;
        }

        //check if one of the two points is in front of the camera
        if (t0 > 0.0) {
            t = t0;
            return true;
        }
        if (t1 > 0.0) {
            t = t1;
            return true;
        }
    }
    t = -1.0; //no intersection
    return false;
}

void main() {
    vec2 normalizedUV = fragUV * 2.0 - 1.0;
    vec4 rayClip = vec4(normalizedUV.x, normalizedUV.y, -1.0, 1.0);
    
    vec4 rayEye = inverse(ubo.projection) * rayClip; 
    rayEye.z = -1.0;
    rayEye.w = 0.0;

    vec3 rayDirection = normalize((inverse(ubo.view) * rayEye).xyz);

    vec3 rayOrigin = (inverse(ubo.view) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;

    float closestT = 1.0e+30; //a big number
    vec4 closestColor = vec4(0.0, 0.0, 0.0, 1.0); //background color

    for (int i = 0; i < 2; ++i) { // Boucle pour chaque sphère
        float t;
        if (sphereHit(rayOrigin, rayDirection, spheres[i], t)) {
            if (t < closestT) {
                closestT = t;
                closestColor = spheres[i].color;
            }
        }
    }

    outColor = closestColor;
}