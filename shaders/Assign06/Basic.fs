#version 410 core

// Change to 410 for macOS

layout(location=0) out vec4 out_color;
 
in vec4 vertexColor; // Now interpolated across face
in vec4 interPos;
in vec3 interNormal;

struct PointLight {
    vec4 pos;
    vec4 color;
};

uniform PointLight light;

void main()
{
    vec3 N = normalize(interNormal); 
    vec3 V = normalize(-interPos.xyz); 

    vec3 L = normalize(vec3(light.pos - interPos)); 

    float diff = max(dot(N, L), 0.0);

    vec3 diffColor = vec3(vertexColor * light.color * diff); 

    float shininess = 10.0;
    vec3 H = normalize(L + V); 
    float specularCoefficient = pow(max(dot(N, H), 0.0), shininess);

    vec3 specularColor = diff * vec3(light.color * specularCoefficient);

    vec3 finalColor = diffColor + specularColor;

    out_color = vec4(finalColor, 1.0f);
}
