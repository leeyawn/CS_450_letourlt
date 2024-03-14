#version 410 core

layout(location=0) in vec3 position;
layout(location=1) in vec4 color;
layout(location=2) in vec3 normal;
layout(location=3) in vec2 texcoord;
layout(location=4) in vec3 tangent;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
uniform mat3 normalMat;

out vec4 interColor;
out vec3 interPos;
out vec3 interNormal;
out vec2 interUV;
out vec3 interTangent;

void main() {
    vec4 pos = vec4(position, 1.0);
    vec4 viewPos = viewMat * modelMat * pos;
    vec4 projPos = projMat * viewPos;

    interColor = color;
    interPos = vec3(viewPos);
    interNormal = normalMat * normal;
    interTangent = vec3(viewMat * modelMat * vec4(tangent, 0.0));

    interUV = texcoord;

    gl_Position = projPos;
}
