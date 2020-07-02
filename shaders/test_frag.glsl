#version 450 core




// Output for on-screen color.
layout(location = 0) out vec4 outColor;
in vec3 WorldPos0;
in vec3 Normal0;

void main()
{

    outColor = vec4(vec3(WorldPos0),1.0);

}