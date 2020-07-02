#version 450 core


// Output for on-screen color.
layout(location = 0) out vec4 outColor;
layout(location = 3) uniform vec3 cameraPos;

in vec3 fragPos; 
in vec3 fragNormal; 

void main()
{
	//Draw the character in red
	outColor = vec4(vec3(1,0,0), 1.0);
}