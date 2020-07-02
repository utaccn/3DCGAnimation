#version 450

// Model/view/projection matrix
layout(location = 0) uniform mat4 mvpMatrix;

layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal; 
layout(location = 2) in vec2 texCoord;

out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragTexCoord;

void main() {
    gl_Position = mvpMatrix * vec4(position, 1.0);
    fragPos = position;
    fragNormal = normal;
    fragTexCoord = texCoord;
}