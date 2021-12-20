// Minimal fragment shader with added fog.
// The fog has been implemented as no fog, linear, exp and exp2
// controlled by a uniform defined in the application

#version 400

in vec4 fcolour;
in float fdistance;
out vec4 outputColor;

// Fog parameters, could make them uniforms and pass them into the shader
float fog_maxdist = 5.0;
float fog_mindist = 0.1;
vec4  fog_colour = vec4(0.4, 0.4, 0.4, 1.0);
const float fogDensity = 0.2;

uniform uint fogmode;
void main()
{
	outputColor = fcolour;

	float fog_factor;
	if (fogmode == 0)
	{
		// No fog
		outputColor = fcolour;
	}
	else 
	{
		// dist used by all fog calculations
		float dist = abs(fdistance);
		
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
		outputColor = mix(fog_colour, fcolour, fog_factor);
	}
}