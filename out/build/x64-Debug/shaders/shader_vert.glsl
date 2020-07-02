#version 450

// Model/view/projection matrix
layout(location = 0) uniform mat4 mvpMatrix;

layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal; 
layout(location = 2) in vec2 texCoord; 
layout(location = 3) uniform mat4 lightMVP;

// Data to pass to fragment shader
out vec3 fragPos;
out vec3 fragNormal;
out vec4 fragLightCoord;
out vec2 fragTexCoord;

void main() {
	// Transform 3D position into on-screen position
    gl_Position = mvpMatrix * vec4(position, 1.0);
    fragPos = position;
    fragNormal = normal;
    fragLightCoord =  lightMVP * vec4(fragPos, 1.0);
    fragTexCoord = texCoord;
}