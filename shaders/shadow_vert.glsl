#version 450
//layout(location = 0) uniform mat4 mvpMatrix;
//layout(location = 0) in vec3 position;
//void main()
//{
//    gl_Position = mvpMatrix * vec4(position, 1);
//}


// Model/view/projection matrix
layout(location = 0) uniform mat4 mvp;

// Per-vertex attributes
layout(location = 0) in vec3 pos; // World-space position
layout(location = 1) in vec3 normal; // World-space normal
layout(location = 3) uniform mat4 lightMVP;

// Data to pass to fragment shader
out vec3 fragPos;
out vec3 fragNormal;

void main() {
	// Transform 3D position into on-screen position
    gl_Position = lightMVP * vec4(pos, 1.0);
    // Pass position and normal through to fragment shader
    fragPos = pos;
    fragNormal = normal;
}

