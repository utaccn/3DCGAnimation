#version 450

layout(location = 0) uniform mat4 mvpMatrix;
layout(location = 3) uniform mat4 lightMVP;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec4 CameraCoord;

void main()
{
    gl_Position = mvpMatrix * vec4(position, 1);

    fragPos = position;
    fragNormal = normal;
    CameraCoord =  mvpMatrix * vec4(fragPos, 1.0);
    fragTexCoord = texCoord;
}