#version 430

//Toon with specularity
layout(location = 4) uniform vec3 cameraPos;
layout(location = 3) uniform vec3 lightPos;

vec3 lightColor = vec3(0.95, 0.5, 0.2);
float threshold = 0.5;
//number of colors
int toonDiscr = 6;
vec3 kD = vec3(0.5);
float shine = 10.0;

layout(location = 0) out vec4 outColor;

in vec3 fragPos; 
in vec3 fragNormal; 

void main()
{
    vec3 viewDir = cameraPos - fragPos;
    vec3 lightDir = lightPos - fragPos;

    //Reflections
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(halfDir, fragNormal), 0.0);
    float specular = pow(specAngle, shine);
    float contr;
    if(specular <= threshold){
    contr = 0.0;
    }
    else {
    contr = 1.0;
    }

    vec3 diocane = normalize(lightDir);
    //Diffuse component to be discretized
    vec3 diffuse = kD*dot(fragNormal, diocane);

    outColor = vec4(lightColor*(floor((diffuse*toonDiscr))/toonDiscr) + lightColor*contr, 1.0);

}