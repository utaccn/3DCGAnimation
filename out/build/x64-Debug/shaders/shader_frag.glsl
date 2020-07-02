#version 450 core

// Global variables for lighting calculations.
layout(location = 8) uniform sampler2D texShadow;
layout(location = 4) uniform vec3 lightPos;
layout(location = 5) uniform vec3 cameraPos;
layout(location = 12) uniform sampler2D wood;

//Parameters for Blinn-Phong shading
vec3 lightColor = vec3(0.95, 0.5, 0.2);
vec3 ambientColor = vec3(0.2,0.2,0.2);
float shine = 9.f;

// Output for on-screen color.
layout(location = 0) out vec4 outColor;

in vec3 fragPos; 
in vec3 fragNormal; 
in vec2 fragTexCoord;
in vec4 fragLightCoord;
vec3 lightCoord;
float bias = 0.001;

const int pcfCount =4;
const float totalTexels = (pcfCount*2.0 +1.0) * (pcfCount*2.0 +1.0);

void main()
{
vec3 lightDir = normalize(lightPos - fragPos);

lightCoord = fragLightCoord.xyz / fragLightCoord.w;
lightCoord = lightCoord.xyz * 0.5 + 0.5;
float fragLightDepth = lightCoord.z;
vec2 shadowMapCoord = lightCoord.xy;

    //PCF shadow
    float mapSize = 1024;
    float texelSize = 1.0/mapSize;
    float total = 0.0;
    for(int x=-pcfCount; x<=pcfCount; x++){
        for(int y=-pcfCount; y<=pcfCount; y++){
        float shadowMapDepth = texture(texShadow, shadowMapCoord + vec2(x,y)*texelSize).x;
        if(shadowMapDepth < fragLightDepth-bias){ total += 1.0;}
        }
    }

        total /=totalTexels;
        float visibility = 1.0 - total;
        float multi = distance(shadowMapCoord, vec2(0.5,0.5));
        
//Blinn-Phong shading values
    vec3 viewDir = cameraPos - fragPos;
    vec3 lightDio = lightPos - fragPos;
    vec3 halfDir = normalize(lightDio + viewDir);
    float specAngle = max(dot(halfDir, fragNormal), 0.0);
    float specular = pow(specAngle, shine);

    //Render light with PCF
    outColor = vec4(texture(wood, fragTexCoord).rgb*visibility*(ambientColor+lightColor*specular*vec3(max(dot(fragNormal,lightDir),0.0))),1.0);

}