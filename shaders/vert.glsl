#version 450

layout (location=0) in vec3 vertexPos;
layout (location=1) in vec2 vertexUV;

layout (location=1) out vec2 fragUV;

void main() {
	fragUV = vertexUV;
	gl_Position = vec4(vertexPos.xyz, 1.0);
}