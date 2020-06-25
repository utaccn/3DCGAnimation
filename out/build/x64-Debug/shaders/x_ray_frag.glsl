#version 430

//Toon with specularity
// Global variables for lighting calculations
layout(location = 5) uniform vec3 cameraPos;
layout(location = 4) uniform vec3 lightPos;
// Global variables for lighting calculations.
layout(location = 8) uniform sampler2D texShadow;
layout(location = 10) uniform sampler2D texToon;

in vec4 fragLightCoord;
vec3 lightCoord;

vec3 lightColor = vec3(0.95, 0.85, 0.5);
float threshold = 0.5;
int toonDiscr = 6;
vec3 kD = vec3(0.5);
float shine = 25.0;
float bias = 0.0001;
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
    float specAngle = dot(normalize(fragNormal), halfDir);
    float specular = pow(specAngle, shine);
    vec3 ambient = kD*dot(normalize(fragNormal),normalize(lightDir));
    vec4 lambert = vec4(ambient*lightColor, 1.0);

	// Divide by w because fragLightCoord are homogeneous coordinates
	lightCoord = fragLightCoord.xyz / fragLightCoord.w;
	// The resulting value is in NDC space (-1 to +1),
	//  we transform them to texture space (0 to 1).
	lightCoord = lightCoord.xyz * 0.5 + 0.5;
	// Depth of the fragment with respect to the light
	float fragLightDepth = lightCoord.z;
	// Shadow map coordinate corresponding to this fragment
	vec2 shadowMapCoord = lightCoord.xy;

	float contr;
	if(specular <= threshold){
		contr = 0.0;
	}
	else {
		contr = 1.0;
	}
	//Discarding Pixels touched by light
	float shadowMapDepth2 = texture(texShadow, shadowMapCoord).x;
	
	//Discard a circle
	//float lecircle = pow(shadowMapCoord.x-0.5,2) + pow(shadowMapCoord.y-0.5,2);
	//if((shadowMapDepth2 > fragLightDepth-bias)&&(lecircle < 0.1)){

	if((shadowMapDepth2 > fragLightDepth-bias)){
		discard;
	}

	//X-toon
	
	float z_dist = gl_FragCoord.z/(gl_FragCoord.w*50);
	outColor = vec4(vec3(z_dist), 1.0);

	outColor = vec4(lambert + texture(texToon, vec2(z_dist, specular)));
}
