#version 430

//Toon with specularity
// Global variables for lighting calculations
layout(location = 4) uniform vec3 cameraPos;
layout(location = 3) uniform vec3 lightPos;

vec3 lightColor = vec3(0.95, 0.5, 0.2);
float threshold = 0.5;
int toonDiscr = 6;
vec3 kD = vec3(0.5);
float shine = 10.0;
// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

void main()
{
    vec3 viewDir = cameraPos - fragPos;
    vec3 lightDir = lightPos - fragPos;

    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(halfDir, fragNormal), 0.0);
    //float specAngle = dot(normalize(fragNormal), halfDir);
    float specular = pow(specAngle, shine);
    float contr;
    if(specular <= threshold){
    contr = 0.0;
    }
    else {
    contr = 1.0;
    }

    vec3 diocane = normalize(lightDir);
    vec3 cane = kD*dot(normalize(fragNormal),diocane);
    vec3 diffuse = kD*dot(fragNormal, diocane);

    outColor = vec4(lightColor*(floor((diffuse*toonDiscr))/toonDiscr) + lightColor*abs(contr*(floor((cane*toonDiscr))/toonDiscr)), 1.0);

}