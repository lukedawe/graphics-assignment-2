// Minimal vertex shader

#version 400

// These are the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;		// Not used
layout(location = 2) in vec3 normal;

// Uniform variables are passed in from the application
uniform mat4 model, view, projection;
uniform uint colourmode;

// Output the vertex colour - to be rasterized into pixel fragments
out vec4 fcolour;
vec4 ambient = vec4(0.2, 0.2,0.2,1.0);
vec3 light_dir = vec3(0.0, 0.0, 10.0);

void main()
{
	vec4 specular_colour = vec4(0.0,0.0,0.0,1.0);
	vec4 diffuse_colour = vec4(colour,1.0);
	vec4 position_h = vec4(position, 1.0);
	float shininess = 8.0;
	
	// Set colours based on height in vertex terrain
	if (colourmode == 0)
	{
		if (position.y <= 0)
		{
			diffuse_colour = vec4(0.2, 0.2, 1.0, 1.0);
		}
		else if (position.y < 1.0)
		{
			diffuse_colour = vec4(0.0, 0.6, 0.2, 1.0);
		}
		else if (position.y < 2.0)
		{
			diffuse_colour = vec4(0.6, 0.4, 0.2, 1.0);
		}
		else
		{
			diffuse_colour = vec4(0.9, 0.8, 0.9, 1.0);
		}
	}

	ambient = diffuse_colour * 0.2;

	mat4 mv_matrix = view * model;
	mat3 normalmatrix = mat3(mv_matrix);
	vec3 N = mat3(mv_matrix) * normal;
	N = normalize(N);
	light_dir = normalize(light_dir);

	vec3 diffuse = max(dot(N, light_dir), 0.0) * diffuse_colour.xyz;

	vec4 P = position_h * mv_matrix;
	vec3 half_vec = normalize(light_dir + P.xyz);
	vec4 specular = pow(max(dot(N, half_vec), 0.0), shininess) * specular_colour;

	// Define the vertex colour
	fcolour = vec4(diffuse,1.0) + ambient + specular;

	// Define the vertex position
	gl_Position = projection * view * model * position_h;
}

