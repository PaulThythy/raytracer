#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;

void main() {
    // Apply the MVP transformation to get the final position
    vec4 worldPos = ubo.model * vec4(inPos, 1.0);
    fragPos = vec3(worldPos);
    fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal; // Correct normals

    gl_Position = ubo.proj * ubo.view * worldPos;
}