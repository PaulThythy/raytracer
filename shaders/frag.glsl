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

void main() {
    outColor = vec4(0.0, 0.0, 0.0, 1.0);
}