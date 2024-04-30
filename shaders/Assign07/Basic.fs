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
uniform float metallic;
uniform float roughness;
const float PI = 3.14159265359;

vec3 getFresnelAtAngleZero(vec3 albedo, float metallic) {
    vec3 F0 = vec3(0.04);

    F0 = mix(F0, albedo, metallic);

    return F0;
}

vec3 getFresnel(vec3 F0, vec3 L, vec3 H) {
    float cosAngle = max(dot(L, H), 0.0);

    return F0 + (1.0 - F0) * pow(1.0 - cosAngle, 5.0);
}
    
float getNDF(vec3 H, vec3 N, float roughness) {
    float alphaSqr = pow(pow(roughness, 2.0), 2.0);
    float NdotHSqr = pow(max(dot(N, H), 0.0), 2.0);
    float denom = (NdotHSqr * (alphaSqr - 1.0) + 1.0);

    return alphaSqr / (PI * denom * denom);
}

float getSchlickGeo(vec3 B, vec3 N, float roughness) {
    float k = pow((roughness + 1.0), 2.0) / 8.0;
    float NdotB = max(dot(N, B), 0.0);

    return NdotB / (NdotB * (1.0 - k) + k);
}

float getGF(vec3 L, vec3 V, vec3 N, float roughness) {
    float GL = getSchlickGeo(L, N, roughness);
    float GV = getSchlickGeo(V, N, roughness);

    return GL * GV;
}

void main()
{
    vec3 N = normalize(interNormal); 
    vec3 V = normalize(-interPos.xyz); 

    vec3 L = normalize(vec3(light.pos - interPos)); 

    vec3 F0 = getFresnelAtAngleZero(vec3(vertexColor), metallic);
    vec3 H = normalize(L + V); 
    vec3 F = getFresnel(F0, L, H);
    vec3 kS = F;

    // diffuse color
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - metallic);

    if (metallic > 0.0) {
       kD = vec3(0.0);
    }
    kD *= vec3(vertexColor) / PI;

    // specular reflection
    float NDF = getNDF(H, N, roughness);
    float G = getGF(L, V, N, roughness);
    kS *= NDF * G;
    kS /= (4.0 * max(0.0, dot(N, L)) * max(0.0, dot(N, V)) + 0.0001);

    float diff = max(dot(N, L), 0.0);

    vec3 diffColor = vec3(vertexColor * light.color * diff); 

    float shininess = 10.0;
    float specularCoefficient = pow(max(dot(N, H), 0.0), shininess);

    vec3 specularColor = vec3(light.color * specularCoefficient);

    vec3 finalColor = (kD + kS) * vec3(light.color) * max(0.0, dot(N, L));

    out_color = vec4(finalColor, 1.0);
}
