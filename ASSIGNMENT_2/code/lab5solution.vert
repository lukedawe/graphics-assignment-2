// Lab5 Solution
// Vertex shader which calculates Gouraud shading
// lighting vectors passes through texture coordinates
// Luke Dawe 2021

#version 420

// These are the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texcoord;

// Uniform variables are passed in from the application
uniform mat4 model, view, projection;
uniform uint colourmode;

// Output the vertex colour - to be rasterized into pixel fragments
out vec4 fcolour;
// Output the  texture coordinate - just pass it through
out vec2 ftexcoord;
// Outputs to send to the fragment shader
out vec3 fnormal;
out vec3 flightdir, fposition;
out vec4 fdiffusecolour;

vec4 ambient = vec4(0.2, 0.2,0.2,1.0);
vec3 light_dir = vec3(0.0, 0.0, 10.0);
// These are the uniforms that are defined in the application
uniform mat3 normalmatrix;
uniform vec4 lightpos;

void main()
{
	vec3 light_pos3 = lightpos.xyz;		

	vec4 specular_colour = vec4(1.0,1.0,1.0,1.0);
	vec4 diffuse_colour = vec4(0.5,0.5,0,1.0);
	vec4 position_h = vec4(position, 1.0);
	float shininess = 8.0;
	
	if (colourmode == 1)
		diffuse_colour = colour;
	else
		diffuse_colour = vec4(1.0, 1.0, 1.0, 1.0);

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
	fcolour = vec4(diffuse, 1.0) + ambient + specular;

	// Define the vertex position
	gl_Position = projection * view * model * position_h;

	// Pass through the texture coordinate
	ftexcoord = texcoord;

	fposition = (mv_matrix * position_h).xyz;			// Modify the vertex position (x, y, z, w) by the model-view transformation
	fnormal = normalize(normalmatrix * normal);	// Modify the normals by the normal-matrix (i.e. to model-view (or eye) coordinates )
	flightdir = light_pos3 - fposition;				// Calculate the vector from the light position to the vertex in eye space

	// Calculate the vertex position in projectin space and output to the pipleline using the reserved variable gl_Position
	gl_Position = (projection * view * model) * position_h;
}

