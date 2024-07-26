#version 450

layout(location=1) in vec2 fragUV;

layout(location=0) out vec4 outColor;

//sphere infos
struct Sphere {
    vec3 center;
    float radius;
    vec4 color;
};

struct Camera {
    vec3 position;
    vec3 lookAt;
    vec3 up;
    float fov;
    float aspectRatio;
    vec3 horizontal;            //horizontal vector of the viewport
    vec3 vertical;              //vertical vector of the viewport
    vec3 lowerLeftCorner;       //lower left corner of the viewport
};


const Sphere spheres[] = {
    Sphere(vec3(0.0, 0.0, -60.0), 1.0, vec4(1.0, 0.0, 0.0, 1.0)),
    Sphere(vec3(0.0, 4.0, -60.0), 0.3, vec4(0.0, 0.0, 1.0, 1.0))
};

Camera calculateCamera(vec3 position, vec3 lookAt, vec3 up, float fov, float aspectRatio) {
    Camera cam;
    float theta = radians(fov);
    float h = tan(theta / 2.0);
    float viewportHeight = 2.0 * h;
    float viewportWidth = aspectRatio * viewportHeight;

    vec3 w = normalize(position - lookAt);
    vec3 u = normalize(cross(up, w));
    vec3 v = cross(w, u);

    cam.position = position;
    cam.horizontal = viewportWidth * u;
    cam.vertical = viewportHeight * v;
    cam.lookAt = lookAt;
    cam.up = up;
    cam.fov = fov;
    cam.aspectRatio = aspectRatio;
    cam.lowerLeftCorner = cam.position - cam.horizontal / 2.0 - cam.vertical / 2.0 - w;

    return cam;
}

vec3 getRayDirection(Camera camera, vec2 uv) {
    return normalize(camera.lowerLeftCorner + uv.x * camera.horizontal + uv.y * camera.vertical - camera.position);
}

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
    vec3 lookFrom = vec3(0.0, 0.0, 0.0);
    vec3 lookAt = vec3(0.0, 0.0, -1.0);
    vec3 vup = vec3(0.0, 1.0, 0.0);
    float fov = 16.0/9.0;
    float aspectRatio = 16.0 / 9.0;

    Camera camera = calculateCamera(lookFrom, lookAt, vup, fov, aspectRatio);

    vec2 normalizedUV = fragUV * 2.0 - 1.0;
    vec3 rayDirection = getRayDirection(camera, normalizedUV);
    vec3 rayOrigin = camera.position;

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