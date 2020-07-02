#version 430

//X-Toon with specularity
layout(location = 5) uniform vec3 cameraPos;
layout(location = 4) uniform vec3 lightPos;
layout(location = 8) uniform sampler2D texDiscard;
layout(location = 10) uniform sampler2D texToon;
layout(location= 20) uniform float radiusX;


in vec4 CameraCoord;
in vec3 fragPos; 
in vec3 fragNormal; 
vec3 lightColor = vec3(0.95, 0.85, 0.5);
float threshold = 0.5;
int toonDiscr = 6;
vec3 kD = vec3(0.5);
float shine = 25.0;
float bias = 0.0001;

layout(location = 0) out vec4 outColor;


void main()
{
	//Blin-phong shading parameters
    vec3 viewDir = cameraPos - fragPos;
    vec3 lightDir = lightPos - fragPos;
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = dot(normalize(fragNormal), halfDir);
    float specular = pow(specAngle, shine);
    vec3 ambient = kD*dot(normalize(fragNormal),normalize(lightDir));
    vec4 lambert = vec4(ambient*lightColor, 1.0);

	//Depth Map texture
	vec3 cameraCoord = CameraCoord.xyz / CameraCoord.w;
	cameraCoord = cameraCoord.xyz * 0.5 + 0.5;
	float cameraDepth = cameraCoord.z;
	vec2 DepthMapCoord = cameraCoord.xy;
	float MapDepth2 = texture(texDiscard, DepthMapCoord).x;
	
	//Discard a circle which radius can be increased/decreased pressing "c"/"v"
	float lecircle = pow(DepthMapCoord.x-0.5,2) + pow(DepthMapCoord.y-0.5,2);
	if((MapDepth2 > cameraDepth-bias)&&(lecircle < radiusX)){
		discard;
	}
	float z_dist = gl_FragCoord.z/(gl_FragCoord.w*50);
	outColor = vec4(lambert + texture(texToon, vec2(z_dist, specular)));
	if(lecircle> radiusX) { outColor = vec4(lambert);}
}
