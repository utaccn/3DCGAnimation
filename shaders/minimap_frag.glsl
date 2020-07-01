#version 450 core

// Global variables for lighting calculations.
layout(location = 10) uniform sampler2D minimap;

// Output for on-screen color.
layout(location = 0) out vec3 outColor;

// Interpolated output data from vertex shader.
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal
in vec2 fragTexCoord;

void main()
{
    outColor = vec3(texture(minimap, fragTexCoord).rgb);
}
