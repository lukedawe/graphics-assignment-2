// Minimal fragment shader

#version 400

// Global constants (for this fragment shader)
vec4 specular_colour = vec4(1.0, 0.8, 0.6, 1.0);
vec4 global_ambient = vec4(0.05, 0.05, 0.05, 1.0);

// Inputs from the vertex shader
in vec3 fnormal, flightdir, fposition;
in vec4 fdiffusecolour, fambientcolour;

uniform uint emitmode, attenuationmode;
uniform float shininess;
uniform float attenuation_k1, attenuation_k2, attenuation_k3;

// Output pixel fragment colour
out vec4 outputColor;
void main()
{
	vec4 emissive = vec4(0);				// Create a vec4(0, 0, 0) for our emmissive light
	vec4 fambientcolour = fdiffusecolour * 0.2;
	float distancetolight = length(flightdir);

	// Normalise interpolated vectors
	vec3 L = normalize(flightdir);
	vec3 N = normalize(fnormal);		

	// Calculate the diffuse component
	vec4 diffuse = max(dot(N, L), 0.0) * fdiffusecolour;

	// Calculate the specular component using Phong specular reflection
	vec3 V = normalize(-fposition.xyz);	
	vec3 R = reflect(-L, N);
	vec4 specular = pow(max(dot(R, V), 0.0), shininess) * specular_colour;

	// Calculate the attenuation factor;
	float attenuation = 1.0;

	if (attenuationmode == 1)
		attenuation = 1.0 / (attenuation_k1 + attenuation_k2*distancetolight + attenuation_k3 * pow(distancetolight, 2));

	// If emitmode is 1 then we enable emmissive lighting
	if (emitmode == 1) emissive = vec4(1.0, 1.0, 0.8, 1.0); 

	// Calculate the output colour, includung attenuation on the diffuse and specular components
	// Note that you may want to exclude the ambient form the attenuation factor so objects
	// are always visible, or include a global ambient
	outputColor = attenuation*(fambientcolour + diffuse + specular) + emissive + global_ambient;

}