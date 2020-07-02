#version 450

// Model/view/projection matrix
layout(location = 0) uniform mat4 mvpMatrix;

// Per-vertex attributes
layout(location = 0) in vec3 position; // World-space position
layout(location = 1) in vec3 normal; // World-space normal
layout(location = 3) uniform mat4 lightMVP;

layout(location = 2) in vec2 texCoord;
// Data to pass to fragment shader
out vec3 fragPos;
out vec3 fragNormal;
out vec4 fragLightCoord;
out vec2 fragTexCoord;

void main() {
    gl_Position = mvpMatrix * vec4(position, 1.0);
    fragPos = position;
    fragNormal = normal;
    fragLightCoord =  lightMVP * vec4(fragPos, 1.0);
    fragTexCoord = texCoord;
}