/*
 foglight.cpp
 Demonstrates a positional light with attenuation and fog with both
 pervertex lighting (Gouraud shading) and per-fragment lighting (Phong shading),
 with fog implemented in both.
 A uniform, fogmode, is used to switch between fog equations.
 Displays a cube and a sphere and a small sphere to show the light position
 Includes controls to move the light source and rotate the view
 Iain Martin October 2019
*/

/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glloadD.lib")
#pragma comment(lib, "opengl32.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include headers for our objects
#include "sphere.h"
#include "cube.h"

/* Define buffer object indices */
GLuint positionBufferObject, colourObject, normalsBufferObject;
GLuint sphereBufferObject, sphereNormals, sphereColours;
GLuint elementbuffer;

GLuint program[2];		/* Identifiers for the shader prgorams */
GLuint current_program;
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */
GLuint emitmode;

GLuint fogmode;		/* Cycle between different fog equations */

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, scaler, z, y, vx, vy, vz;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;	//Define the resolution of the sphere object

GLfloat light_x, light_y, light_z;

/* Uniforms*/
GLuint modelID[2], viewID[2], projectionID[2], lightposID[2], normalmatrixID[2];
GLuint colourmodeID[2], emitmodeID[2], fogmodeID[2];

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;

Cube aCube;
Sphere aSphere;

using namespace std;
using namespace glm;

// Define fogmode strings to output to screen
string fog_mode_desc[] = {"off", "linear", "exp", "exp2" };

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	/* Set the object transformation controls to their initial values */
	x = 0.05f;
	y = 0;
	z = 0;
	vx = 0; vx = 0, vz = 4.f;
	light_x = 0; light_y = 0; light_z = 0;
	angle_x = angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	scaler = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 0; emitmode = 0;
	fogmode = 0;
	numlats = 20;		// Number of latitudes in our sphere
	numlongs = 20;		// Number of longitudes in our sphere

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program[0] = glw->LoadShader("poslight_fog.vert", "poslight_fog.frag");
		program[1] = glw->LoadShader("fraglight_fog.vert", "fraglight_fog.frag");
	}
	catch (exception &e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define the same uniforms to send to both shaders */
	for (int i = 0; i < 2; i++)
	{
		glUseProgram(program[i]);
		modelID[i] = glGetUniformLocation(program[i], "model");
		colourmodeID[i] = glGetUniformLocation(program[i], "colourmode");
		emitmodeID[i] = glGetUniformLocation(program[i], "emitmode");
		viewID[i] = glGetUniformLocation(program[i], "view");
		projectionID[i] = glGetUniformLocation(program[i], "projection");
		lightposID[i] = glGetUniformLocation(program[i], "lightpos");
		normalmatrixID[i] = glGetUniformLocation(program[i], "normalmatrix");
		fogmodeID[i] = glGetUniformLocation(program[i], "fogmode");
	}

	// Define the index which represents the current shader (i.e. default is gouraud)
	current_program = 0;

	/* create our sphere and cube objects */
	aSphere.makeSphere(numlats, numlongs);
	aCube.makeCube();
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Make the compiled shader program current */
	glUseProgram(program[current_program]);

	/* Bind cube vertices. Note that this is in attribute index 0 */
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind cube colours. Note that this is in attribute index 1 */
	glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind cube normals. Note that this is in attribute index 2 */
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBufferObject);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Define the model transformations for the cube
	mat4 model = mat4(1.0f);
	model = translate(model, vec3(x+0.5, y, z));
	model = scale(model, vec3(scaler, scaler, scaler));//scale equally in all axis
	model = rotate(model, -radians(angle_x), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = rotate(model, -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = rotate(model, -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, 0, 4), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// Apply rotations to the view position
	view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = rotate(view, -radians(vy), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = rotate(view, -radians(vz), vec3(0, 0, 1));

	// Define the light position and transform by the view matrix
	vec4 lightpos = view *  vec4(light_x, light_y, light_z, 1.0);

	// Define the normal matrix
	mat3 normalmatrix = transpose(inverse(mat3(view * model)));

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &model[0][0]);
	glUniform1ui(colourmodeID[current_program], colourmode);
	glUniform1ui(emitmodeID[current_program], emitmode);
	glUniform1ui(fogmodeID[current_program], fogmode);
	glUniformMatrix4fv(viewID[current_program], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID[current_program], 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);
	glUniform4fv(lightposID[current_program], 1, value_ptr(lightpos));

	/* Draw our cube*/
	aCube.drawCube(drawmode);

	/* Define the model transformations for our sphere */
	model = mat4(1.0f);
	model = translate(model, vec3(-x-0.5, 0, 0));
	model = scale(model, vec3(scaler/3.f, scaler/3.f, scaler/3.f));//scale equally in all axis
	model = rotate(model, -radians(angle_x), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = rotate(model, -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = rotate(model, -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
	glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &model[0][0]);

	/* Draw our sphere */
	normalmatrix = transpose(inverse(mat3(view * model)));
	glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);
	aSphere.drawSphere(drawmode);

	/* Draw a small sphere in the lightsource position to visually represent the light source */
	model = mat4(1.0f);
	model = translate(model, vec3(light_x, light_y, light_z));
	model = scale(model, vec3(0.05f, 0.05f, 0.05f)); // make a small sphere
	normalmatrix = transpose(inverse(mat3(view * model)));
	glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);
	glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &model[0][0]);

	/* Draw our lightposition sphere */
	emitmode = 1;
	glUniform1ui(emitmodeID[current_program], emitmode);
	aSphere.drawSphere(drawmode);
	emitmode = 0;

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f*4.f) / ((float)h / 480.f*3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == 'Q') angle_inc_x -= 0.05f;
	if (key == 'W') angle_inc_x += 0.05f;
	if (key == 'E') angle_inc_y -= 0.05f;
	if (key == 'R') angle_inc_y += 0.05f;
	if (key == 'T') angle_inc_z -= 0.05f;
	if (key == 'Y') angle_inc_z += 0.05f;
	if (key == 'A') scaler -= 0.02f;
	if (key == 'S') scaler += 0.02f;
	if (key == 'Z') x -= 0.05f;
	if (key == 'X') x += 0.05f;
	if (key == 'C') y -= 0.05f;
	if (key == 'V') y += 0.05f;
	if (key == 'B') z -= 0.05f;
	if (key == 'N') z += 0.05f;
	if (key == '1') light_x -= 0.05f;
	if (key == '2') light_x += 0.05f;
	if (key == '3') light_y -= 0.05f;
	if (key == '4') light_y += 0.05f;
	if (key == '5') light_z -= 0.05f;
	if (key == '6') light_z += 0.05f;
	if (key == '7') vx -= 1.f;
	if (key == '8') vx += 1.f;
	if (key == '9') vy -= 1.f;
	if (key == '0') vy += 1.f;
	if (key == 'O') vz -= 1.f;
	if (key == 'P') vz += 1.f;

	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == ',' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}

	/* Switch between the vertex lighting shaders and the fragment shader */
	if (key == 'I' && action != GLFW_PRESS)
	{
		current_program++;
		if (current_program == 2) current_program = 0;
	}

	/* Cycle between fogmodes */
	/* Switch between the vertex lighting shaders and the fragment shader */
	if (key == 'F' && action != GLFW_PRESS)
	{
		fogmode == 3 ? fogmode = 0 : fogmode++;
		cout << "Fogmode: " << fog_mode_desc[fogmode] << endl;
	}
}



/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Lab2: Hello 3D");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}


