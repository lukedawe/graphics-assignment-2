// Minimal fragment shader

#version 400

in vec4 fcolour;
out vec4 outputColor;
void main()
{
	outputColor = fcolour;
	//outputColor = vec4(1.0,0.0,0.0,1.0);
}