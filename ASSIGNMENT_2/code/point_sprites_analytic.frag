// Fragment shader designed for Point Sprites
// The shape of the point becomes a circle by discarding the bits of 
// it greater than a given radius from the centre of the point.
// Iain Martin Novemebr 2018
// Addapted from Example 6.1 in the Redbook V4.3

#version 420

in vec4 fcolour;
out vec4 outputColor;

void main()
{
	const vec4 colour2 = vec4(0.9, 0.7, 1.0, 0.0);
	vec2 temp = gl_PointCoord - vec2(0.5);
	float f = dot(temp, temp);
	if (f>0.25) discard;
	
	// vec4 texcolour = texture(tex1, gl_PointCoord);
	outputColor = mix (fcolour, colour2, smoothstep(0.01, 0.25, f));
}