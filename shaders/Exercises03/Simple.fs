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

void main() {
    vec3 N = normalize(interNormal);

    vec3 lightPos = vec3(light.pos);
    vec3 L = lightPos - interPos;
    float dist = length(L);
    L = normalize(L);
    float att = 1.0 / (dist * dist + 1);

    //out_color = interColor; //vec4(1.0, 0.0, 0.0, 1.0);
    //out_color = vec4(att, att, att, 1.0);
    //vec3 sN = (N + 1.0) / 2.0;
    //out_color =  vec4(sN, 1.0);

    //L = vec3(0, 0, 1.0);

    float diff = max(dot(N, L), 0.0);
    vec3 diff_color = diff * vec3(interColor);
    //out_color = vec4(diff, diff, diff, 1.0);
    out_color = vec4(diff_color, 1.0);
}
