#version 450 core


// Output for on-screen color.
layout(location = 0) out vec4 outColor;
layout(location = 3) uniform vec3 cameraPos;

// Interpolated output data from vertex shader.
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

float circle(in vec2 _st, in float _radius){
    vec2 dist = _st-vec2(0.5);
	return 1.-smoothstep(_radius-(_radius*0.01),
                         _radius+(_radius*0.01),
                         dot(dist,dist)*4.0);
}

void main()
{

	outColor = vec4(vec3(1,0,0), 1.0);
}