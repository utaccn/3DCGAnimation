#version 450 core

// Minimap texture
layout(location = 10) uniform sampler2D minimap;

// Output for on-screen color.
layout(location = 0) out vec3 outColor;

in vec3 fragPos; 
in vec3 fragNormal; 

void main()
{
    outColor = vec3(texture(minimap, fragPos.xy).rgb);
}
