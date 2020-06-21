#version 430

// Global variables for lighting calculations
layout(location = 3) uniform vec3 cameraPos;
layout(location = 4) uniform vec3 lightPos;

vec3 lightColor = vec3(0.95, 0.85, 0.5);
vec3 ks = vec3(0.1);
float shine = 1.5f;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPosition; // World-space position
in vec3 fragNormal; // World-space normal

void main()
{
    vec3 viewDir = cameraPos - fragPosition;
    vec3 lightDir = lightPos - fragPosition;
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = dot(normalize(fragNormal), halfDir);
    float specular = pow(specAngle, shine);
    // Output the normal as color
    outColor = vec4(abs(specular*lightColor), 1.0);
}
