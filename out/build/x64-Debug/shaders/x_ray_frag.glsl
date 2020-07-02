#version 430

//Toon with specularity
// Global variables for lighting calculations
layout(location = 5) uniform vec3 cameraPos;
layout(location = 4) uniform vec3 lightPos;
// Global variables for lighting calculations.
layout(location = 8) uniform sampler2D texDiscard;
layout(location = 10) uniform sampler2D texToon;


in vec4 CameraCoord;
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
vec3 cameraCoord = CameraCoord.xyz / CameraCoord.w;
cameraCoord = cameraCoord.xyz * 0.5 + 0.5;
float cameraDepth = cameraCoord.z;
vec2 shadowMapCoord = cameraCoord.xy;

	//Discarding Pixels touched by light
	float shadowMapDepth2 = texture(texDiscard, shadowMapCoord).x;
	
	//Discard a circle
	float lecircle = pow(shadowMapCoord.x-0.5,2) + pow(shadowMapCoord.y-0.5,2);
	if((shadowMapDepth2 > cameraDepth-bias)&&(lecircle < 0.05)){
/*
	if((shadowMapDepth2 > cameraDepth-bias)){*/
		discard;
	}

	//X-toon
	
	float z_dist = gl_FragCoord.z/(gl_FragCoord.w*50);
	//outColor = vec4(vec3(z_dist), 1.0);
	outColor = vec4(lambert + texture(texToon, vec2(z_dist, specular)));
}
