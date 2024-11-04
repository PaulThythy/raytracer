#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main() {
    // Set the output color to red
    outColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color with full opacity
}