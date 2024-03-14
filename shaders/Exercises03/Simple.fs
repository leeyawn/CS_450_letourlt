#version 410 core

layout(location=0) out vec4 out_color;

in vec4 interColor;
in vec3 interPos;
in vec3 interNormal;

struct PointLight {
    vec4 pos;
    vec4 color;
};

uniform PointLight light;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
in vec2 interUV;
in vec3 interTangent;

void main() {
    vec3 N = normalize(interNormal);
    vec3 T = normalize(interTangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = normalize(cross(N, T));

    vec3 texN = vec3(texture(normalTexture, interUV));
    texN.x = (texN.x - 0.5) * 2.0;
    texN.y = (texN.y - 0.5) * 2.0;
    texN = normalize(texN);

    mat3 toView = mat3(T,B,N);

    N = normalize(toView * texN);

    vec4 texColor = texture(diffuseTexture, interUV);

    vec3 lightPos = vec3(light.pos);
    vec3 L = lightPos - interPos;
    float dist = length(L);
    L = normalize(L);
    float att = 1.0 / (dist * dist + 1);

    float diff = max(dot(N, L), 0.0);
    vec3 diff_color = diff * vec3(interColor);
    out_color = vec4(diff_color, 1.0);
    //out_color = vec4(texN, 1.0);
    //out_color = vec4(diff, diff, diff, 1.0);
}
