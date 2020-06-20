#version 450 core

// Global variables for lighting calculations.
layout(location = 2) uniform sampler2D texShadow;
layout(location = 4) uniform vec3 lightPos;

// Output for on-screen color.
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader.
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

in vec4 fragLightCoord;
vec3 lightCoord;
float bias = 0.00005;

const int pcfCount =2;
const float totalTexels = (pcfCount*2.0 +1.0) * (pcfCount*2.0 +1.0);

void main()
{
    // Output the normal as color.
    vec3 lightDir = normalize(lightPos - fragPos);

// Divide by w because fragLightCoord are homogeneous coordinates
lightCoord = fragLightCoord.xyz / fragLightCoord.w;
// The resulting value is in NDC space (-1 to +1),
//  we transform them to texture space (0 to 1).
lightCoord = lightCoord.xyz * 0.5 + 0.5;
// Depth of the fragment with respect to the light
float fragLightDepth = lightCoord.z;
// Shadow map coordinate corresponding to this fragment
vec2 shadowMapCoord = lightCoord.xy;

/////////////////////////////////Basic Shadow//////////////////////////
// Shadow map value from the corresponding shadow map position

//float shadowMapDepth = texture(texShadow, shadowMapCoord).x;
//    float visibility = 1.0;
//    if(shadowMapDepth < fragLightDepth-bias){
//    visibility = 0.0;
//    }

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
        
    //Render Spot light with PCF
    outColor = vec4(visibility*vec3(max(dot(fragNormal,lightDir),0.0)),1.0);

}