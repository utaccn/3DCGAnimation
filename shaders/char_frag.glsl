#version 450 core


// Output for on-screen color.
layout(location = 0) out vec4 outColor;
layout(location = 15) uniform sampler2D tex;

in vec3 fragPos; 
in vec3 fragNormal; 
in vec2 fragTexCoord;

void main()
{
	//Draw the texture on top of the character
	outColor = vec4(texture(tex, fragTexCoord).rgb, 1.0);
}