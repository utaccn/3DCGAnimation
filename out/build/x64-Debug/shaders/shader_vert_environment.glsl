#version 450
/*
layout(location = 0) uniform mat4 mvpMatrix;
layout(location = 1) uniform mat4 modelMatrix;
// Normals should be transformed differently than positions:
// https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
layout(location = 2) uniform mat3 normalModelMatrix;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec4 fragLightCoord;

void main()
{
    gl_Position = mvpMatrix * vec4(position, 1);
    
    fragPosition = (modelMatrix * vec4(position, 1)).xyz;
    fragNormal = normalModelMatrix * normal;
    fragTexCoord = texCoord;
} */

//
//
// NEW //

// Model/view/projection matrix
layout(location = 0) uniform mat4 mvp;

// Per-vertex attributes
layout(location = 0) in vec3 pos; // World-space position
layout(location = 1) in vec3 normal; // World-space normal
layout(location = 3) uniform mat4 lightMVP;


// Data to pass to fragment shader
out vec3 fragPosition;
out vec3 fragNormal;
out vec4 fragLightCoord;
void main() {
	// Transform 3D position into on-screen position
    gl_Position = mvp * vec4(pos, 1.0);
    // Pass position and normal through to fragment shader
    fragPosition = pos;
    fragNormal = normal;
    fragLightCoord =  lightMVP * vec4(fragPosition, 1.0);
}
