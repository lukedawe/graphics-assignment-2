// Lab5 Solution
// Fragment shader which combines colour and texture.
// The colour is the lighting colour outputted by the vertex shader
// Luke Dawe 2021

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

	vec4 emissive = vec4(0);				// Create a vec4(0, 0, 0) for our emmissive light
	vec4 fambientcolour = texcolour * 0.2;
	vec4 fspecularcolour =  vec4(1.0, 1.0, 0.5, 1.0);
	float distancetolight = length(flightdir);

	// Normalise interpolated vectors
	vec3 L = normalize(flightdir);
	vec3 N = normalize(fnormal);		

	// Calculate the diffuse component
	vec4 diffuse = max(dot(N, L), 0.0) * texcolour;

	// Calculate the specular component using Phong specular reflection
	vec3 V = normalize(-fposition.xyz);	
	vec3 R = reflect(-L, N);
	vec4 specular = pow(max(dot(R, V), 0.0), shininess) * fspecularcolour;

	// Calculate the attenuation factor;
	float attenuation_k1 = 0.5;
	float attenuation_k2 = 0.5;
	float attenuation_k3 = 0.5;
    float attenuation = 1.0 / (attenuation_k1 + attenuation_k2*distancetolight + attenuation_k3 * pow(distancetolight, 2));
	
	// simple hack to make the light brighter, it would be better to change the attenuation equation!
	attenuation *= abs(-1.5);

	// If emitmode is 1 then we enable emmissive lighting
	if (emitmode == 1) emissive = vec4(1.0, 1.0, 1.0, 1.0); 

	// Calculate the output colour, includung attenuation on the diffuse and specular components
	// Note that you may want to exclude the ambient form the attenuation factor so objects
	// are always visible, or include a global ambient
	vec4 shadedColor = attenuation*(fambientcolour + diffuse + specular) + emissive + global_ambient;
	
	// Calculate linear fog
	float fog_factor;
	outputColor = shadedColor;
	if (fogmode == 0)
	{
		// No fog
	}
	else 
	{
		// dist used by all fog calculations
		float dist = length(fposition.xyz);
		
		if (fogmode == 1){
			// linear fog
			fog_factor = (fog_maxdist-dist) / (fog_maxdist - fog_mindist);
		}
		else if (fogmode == 2) {
			// exponential fog
			fog_factor = 1.0 / exp(dist  * fogDensity);
		}
		else{
			// exponential squared fog
			fog_factor = 1.0 / pow(exp(dist  * fogDensity), 2.0);
		}

		// Limit fog factor to range 0 to 1
		fog_factor = clamp(fog_factor, 0.0, 1.0);

		// Mix the fog with lit pixel fragment colour
		outputColor = mix(fog_colour, shadedColor, fog_factor);
	}
}