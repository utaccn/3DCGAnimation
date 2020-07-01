#version 450

// Model/view/projection matrix
//layout(location = 0) uniform mat4 mvpMatrix;

// Per-vertex attributes
layout(location = 0) in vec3 position; // World-space position
layout(location = 1) in vec3 normal; // World-space normal
layout(location = 0) in vec2 texCoord;
// Data to pass to fragment shader
out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragTexCoord;

void main() {
	// Transform 3D position into on-screen position
    //gl_Position = mvpMatrix * vec4(position, 1.0);
    // Pass position and normal through to fragment shader

    gl_Position = vec4(position, 1.0);
    fragPos = position;
    fragNormal = normal;
    fragTexCoord = texCoord;
}
/*

#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 0) in vec2 uv;

// will be used in fragment shader
out vec2 uv_frag;

void main(){
    uv_frag = uv;
    gl_Position = vec4(vertex, 1.0);
}*/