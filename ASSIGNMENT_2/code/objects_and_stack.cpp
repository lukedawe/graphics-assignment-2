/*
 objects_and_stack.cpp
 Demonstrates the use of the matrix stack to control a scene
 with a cube, a sphere and a tetrahedron with added geometry shader to show normals.
 The stack is used to provide controls common to all objects and sperate controls
 for the individual components.
 The cube, sphere and tetrahedron objects have been seperated out into seperate classes
 to make the code required for each a bit clearer and to simplify the code in the display()
 and init() functions.
 Also displays a small sphere to show the light position
 Includes controls to move the light source, rotate the view, move the object and switch
 between vertex and fragment lighting models.
 Iain Martin November 2017
*/

/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include "tetrahedron.h"
#include "sphere.h"
#include "cube.h"
#include "tiny_loader_texture.h"
#include <iostream>
#include <stack>

/* Include GLM core and matrix extensions*/
//#define GLM_FORCE_RADIANS  // add this line to remove the compiler warning nu then use radians!
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

GLuint program[2];		/* Identifiers for the shader programs */
GLuint current_program;
GLuint show_normals_program; /* Identifiers for the geometry shader program*/

GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */

GLuint attenuationmode;	/* Turns attenuation on and off */

GLfloat emitmode;

GLfloat shininess;	/* specular shininess parameter */

GLfloat k1, k2, k3;	/* Attenuation constants */

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, model_scale, z, y, vx, vy, vz;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLfloat spin, spin_inc;	// Spin sphere and tetrahedron
GLuint drawmode;			// Defines drawing mode as points, lines or filled polygons
GLuint numlats, numlongs;	// Define the resolution of the sphere object

bool show_normals = false;

GLfloat light_x, light_y, light_z;

/* Uniforms*/
GLuint modelID[2], viewID[2], projectionID[2], lightposID[2], normalmatrixID[2];
GLuint colourmodeID[2], emitmodeID[2], shininessID[2], k1ID[2], k2ID[2], k3ID[3], attenuationmodeID[2];
GLuint modelID_sn, viewID_sn, projectionID_sn;	// model, view & projection for the show_normals shader

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/

Tetrahedron tetrahedron;
Sphere sphere;
Cube cube;
TinyObjLoader monkey;

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
	light_x = 0; light_y = 0.25f; light_z = 0.25f;
	angle_x = angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 1; emitmode = 0, attenuationmode=0;
	numlats = 20;		// Number of latitudes in our sphere
	numlongs = 20;		// Number of longitudes in our sphere
	shininess = 8.f;
	k1 = 1.0, k2 = 0, k3 = 1.0; // Set default attenuation parameters
	spin = 0, spin_inc;

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* create the sphere, cube and tetrahedron objects */
	sphere.makeSphere(numlats, numlongs);
	tetrahedron.defineTetrahedron();
	cube.makeCube();

	/* Load and create our monkey object*/
	monkey.load_obj("monkey.obj", true);
	monkey.drawObject(1);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program[0] = glw->LoadShader("poslight.vert", "poslight.frag");
		program[1] = glw->LoadShader("fraglight.vert", "fraglight.frag");

		// Load the shader which draws vertex normals as lines
		show_normals_program = glw->LoadShader("show_normals.vert", 
			"show_normals.frag", 
			"show_normals.geom");
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
		attenuationmodeID[i] = glGetUniformLocation(program[i], "attenuationmode");
		emitmodeID[i] = glGetUniformLocation(program[i], "emitmode");
		shininessID[i] = glGetUniformLocation(program[i], "shininess");
		k1ID[i] = glGetUniformLocation(program[i], "attenuation_k1");
		k2ID[i] = glGetUniformLocation(program[i], "attenuation_k2");
		k3ID[i] = glGetUniformLocation(program[i], "attenuation_k3");
		viewID[i] = glGetUniformLocation(program[i], "view");
		projectionID[i] = glGetUniformLocation(program[i], "projection");
		lightposID[i] = glGetUniformLocation(program[i], "lightpos");
		normalmatrixID[i] = glGetUniformLocation(program[i], "normalmatrix");
	}

	// Define uniforms to send to the show_normals geometry shader program
	glUseProgram(show_normals_program);
	modelID_sn = glGetUniformLocation(show_normals_program, "model");
	viewID_sn = glGetUniformLocation(show_normals_program, "view");
	projectionID_sn = glGetUniformLocation(show_normals_program, "projection");

	// Define the index which represents the current shader (i.e. default is gouraud)
	current_program = 0;
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Make the compiled shader program current */
	glUseProgram(program[current_program]);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, 0, 4), // Camera is at (4,3,3), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// Apply rotations to the view position
	view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = rotate(view, -radians(vy), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = rotate(view, -radians(vz), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	// Define the light position and transform by the view matrix
	vec4 lightpos = view *  vec4(light_x, light_y, light_z, 1.0);

	// Send our common uniforms variables to the currently bound shader,
	// We change the model and light position for some of the other objects
	glUniform1ui(colourmodeID[current_program], colourmode);
	glUniform1ui(attenuationmodeID[current_program], attenuationmode);
	glUniform1ui(emitmodeID[current_program], emitmode);
	glUniform1f(shininessID[current_program], shininess);
	glUniform1f(k1ID[current_program], k1);
	glUniform1f(k1ID[current_program], k2);
	glUniform1f(k1ID[current_program], k3);
	glUniformMatrix4fv(viewID[current_program], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID[current_program], 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID[current_program], 1, value_ptr(lightpos));
	
	// Define our model transformations as a stack object
	mat3 normalmatrix;
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Draw the light source position as a small sphere
	model.push(model.top());
	{
		/* Draw a small sphere in the lightsource position to visually represent the light source */
		model.top() = translate(model.top(), vec3(light_x, light_y, light_z));
		model.top() = scale(model.top(), vec3(0.05f, 0.05f, 0.05f)); // make a small sphere
		normalmatrix = transpose(inverse(mat3(view * model.top())));

		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		/* Draw our lightposition sphere */
		emitmode = 1;
		glUniform1ui(emitmodeID[current_program], emitmode);
		sphere.drawSphere(drawmode);
		emitmode = 0;
		glUniform1ui(emitmodeID[current_program], emitmode);
	}
	model.pop();

	// Define the model transformations for the main object (containing a cube, a sphere and a tetrahedron)
	model.top() = translate(model.top(), vec3(x, y, z));
	model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis
	model.top() = rotate(model.top(), -radians(angle_x), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model.top() = rotate(model.top(), -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model.top() = rotate(model.top(), -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	/* Draw a cube to represent the main object block on to which the sphere and tetrahedron should spin*/
	/* Define the transformations, send the uniforms to the shaders and draw the cube*/
	model.push(model.top());
	{
		// Cube specific transformations
		model.top() = scale(model.top(), vec3(3.f, 1.0f, 1.0f)); // make a squished cube ground floor
		normalmatrix = transpose(inverse(mat3(view * model.top())));

		// Send our current model and normal matrice uniforms variables to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		// Draw our cube
		cube.drawCube(drawmode);

		// Draw our cube as GL_POINTS with show_normals geometry shader
		if (show_normals)
		{
			glUseProgram(show_normals_program);
				glUniformMatrix4fv(modelID_sn, 1, GL_FALSE, &(model.top()[0][0]));
				glUniformMatrix4fv(viewID_sn, 1, GL_FALSE, &view[0][0]);
				glUniformMatrix4fv(projectionID_sn, 1, GL_FALSE, &projection[0][0]);
				cube.drawCube(2);
			glUseProgram(program[current_program]);
		}
	}
	model.pop();

	// Draw the sphere, spinning around it's y-axis
	model.push(model.top());
	{
		/* Define the model transformations for our sphere */
		model.top() = translate(model.top(), vec3(-0.5f, 0.45, 0));
		model.top() = scale(model.top(), vec3(0.2f, 0.2f, 0.2f));//scale equally in all axis
		normalmatrix = transpose(inverse(mat3(view * model.top())));

		// Send our current model and normal matrice uniforms variables to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our tetrahedron object, sitting on top of the cube and spinning */
		sphere.drawSphere(drawmode);

		// Draw our sphere as GL_POINTS with show_normals geometry shader
		if (show_normals)
		{
			glUseProgram(show_normals_program);
				glUniformMatrix4fv(modelID_sn, 1, GL_FALSE, &(model.top()[0][0]));
				glUniformMatrix4fv(viewID_sn, 1, GL_FALSE, &view[0][0]);
				glUniformMatrix4fv(projectionID_sn, 1, GL_FALSE, &projection[0][0]);
				sphere.drawSphere(2);
			glUseProgram(program[current_program]);
		}
	}
	model.pop();

	// Draw the tetrahedron, spinning around it's y-axis
	model.push(model.top());
	{
		/* Define the model transformations for our tetrahedron */
		model.top() = translate(model.top(), vec3(0.5f, 0.25f, 0.f));
		model.top() = scale(model.top(), vec3(0.5f, 0.5f, 0.5f));//scale equally in all axis
		model.top() = rotate(model.top(), radians(spin/2.f), vec3(0, 1, 0)); //rotating in anti-clockwise direction spin around y-axis
		normalmatrix = transpose(inverse(mat3(view * model.top())));

		// Send our current model and normal matrice uniforms variables to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our tetrahedron object, sitting on top of the cube and spinning */
		tetrahedron.drawTetrahedron(drawmode);

		// Draw our tetrahedron as GL_POINTS with show_normals geometry shader
		if (show_normals)
		{
			glUseProgram(show_normals_program);
				glUniformMatrix4fv(modelID_sn, 1, GL_FALSE, &(model.top()[0][0]));
				glUniformMatrix4fv(viewID_sn, 1, GL_FALSE, &view[0][0]);
				glUniformMatrix4fv(projectionID_sn, 1, GL_FALSE, &projection[0][0]);
				tetrahedron.drawTetrahedron(2);
			glUseProgram(program[current_program]);
		}
	}
	model.pop();

	// Draw the WaveFront object, spinning around it's y-axis
	model.push(model.top());
	{
		/* Define the model transformations for our object */
		model.top() = glm::translate(model.top(), glm::vec3(0, 0.4f, 0.f));
		model.top() = glm::scale(model.top(), glm::vec3(0.2f, 0.2f, 0.2f));//scale equally in all axis
		model.top() = glm::rotate(model.top(), -radians(spin/4.f), glm::vec3(0, 1, 0)); //rotating in anti-clockwise direction spin around y-axis
		normalmatrix = glm::transpose(glm::inverse(glm::mat3(view * model.top())));

		// Send our current model and normal matrice uniforms variables to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our wavefront object, sitting on top of the cube and spinning */
		monkey.drawObject(drawmode);

		// Draw object as GL_POINTS with show_normals geometry shader
		if (show_normals)
		{
			glUseProgram(show_normals_program);
				glUniformMatrix4fv(modelID_sn, 1, GL_FALSE, &(model.top()[0][0]));
				glUniformMatrix4fv(viewID_sn, 1, GL_FALSE, &view[0][0]);
				glUniformMatrix4fv(projectionID_sn, 1, GL_FALSE, &projection[0][0]);
				monkey.drawObject(2);
			glUseProgram(program[current_program]);
		}
	}
	model.pop();

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;
	spin += spin_inc;
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
	if (key == 'A') model_scale -= 0.02f;
	if (key == 'S') model_scale += 0.02f;
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
	if (key == 'K') shininess /= 1.2f;
	if (key == 'L') shininess *= 1.2f;
	if (key == 'D') spin_inc += 0.2f;
	if (key == 'F') spin_inc -= 0.2f;

	// Attenuation 
	if (key == '.')
	{
		k1 /= 1.1f;
		k2 /= 1.1f;
		k3 /= 1.2f;
	}
	if (key == '/')
	{
		k1 *= 1.1f;
		k2 *= 1.1f;
		k3 *= 1.2f;
	}

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

	if (key == '-' && action != GLFW_PRESS)
	{
		attenuationmode = !attenuationmode;
		cout << "attenuationmode=" << attenuationmode << endl;
	}

	if (key == '=' && action != GLFW_PRESS)
	{
		show_normals = !show_normals;
	}
}



/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Show normals geometry shader. Press '=' key");

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




