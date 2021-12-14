// Lab5 Solution
// Fragment shader which combines colour and texture.
// The colour is the lighting colour outputted by the vertex shader
// Iain Martin 2019

#version 420

// Fog parameters, could make them uniforms and pass them into the shader
const float fog_maxdist = 5.0;
const float fog_mindist = 0.1;
//const vec4  fog_colour = vec4(0.4, 0.4, 0.4, 1.0);
 const vec4  fog_colour = vec4(0.0, 0.0, 0.0, 1.0);
const float fogDensity = 0.2; 

// Global constants (for this vertex shader)
vec4 specular_colour = vec4(1.0, 0.8, 0.6, 1.0);
vec4 global_ambient = vec4(0.05, 0.05, 0.05, 1.0);
int  shininess = 8;

// Inputs from the vertex shader
in vec3 fnormal, flightdir, fposition;
in vec4 fdiffusecolour, fambientcolour;

//in float distancetolight;

uniform uint emitmode;
uniform uint fogmode;

in vec4 fcolour;
out vec4 outputColor;
in vec2 ftexcoord;

uniform sampler2D tex1;	

void main()
{
	// Extract the texture colour to colour our pixel
	vec4 texcolour = texture(tex1, ftexcoord);
	outputColor = texcolour;
}