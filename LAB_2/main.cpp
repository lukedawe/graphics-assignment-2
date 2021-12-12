/*
 main.cpp
 Shows a music box that can open and spin the angel
 Includes controls to move the light source and rotate the view
 Luke Dawe 2021
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

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>
#include <stack>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include headers for our objects
#include "sphere.h"
#include "cube.h"
#include "cylinder.h"
#include"prism.h"

// for the model
#include "tiny_loader.h"
TinyObjLoader monkey;			// This is an instance of our basic object loaded

/* Define buffer object indices */
GLuint elementbuffer;

GLuint program;		/* Identifier for the shader prgoram */
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */
GLuint emitmode;
GLuint attenuationmode; 


GLboolean open_lid;

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, model_scale, z, y, vx, vy, vz;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;	//Define the resolution of the sphere object
GLfloat speed;				// movement increment

// variables for the rotation of the angel inside the box
GLfloat angel_speed;
GLfloat angle_angle;

// variables for the lid opening
GLfloat lid_angle;

// variables for the light rising
GLfloat light_height;
GLfloat light_rise_speed;

GLfloat light_x, light_y, light_z;

/* Uniforms*/
GLuint modelID, viewID, projectionID, lightposID, normalmatrixID;
GLuint colourmodeID, emitmodeID, attenuationmodeID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;

/* Global instances of our objects */
Sphere aSphere;
Cube aCube;
Cylinder aCylinder;
prism aprism;

using namespace std;
using namespace glm;


static void draw_cube(stack<mat4> model, mat3 normalmatrix, mat4 view, GLfloat translate_x = 0.f, GLfloat translate_y = 0.f, GLfloat translate_z = 0.f, GLfloat scale_x = 0.f, GLfloat scale_y = 0.f, GLfloat scale_z = 0.f, GLfloat rotate_x = 0.f, GLfloat rotate_y = 0.f, GLfloat rotate_z = 0.f, GLfloat rotate_angle = 0.f) {
	// This block of code draws the cube
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(translate_x, translate_y, translate_z));

		// rotate the model 90 degrees
		model.top() = rotate(model.top(), radians(rotate_angle), vec3(rotate_x, rotate_y, rotate_z));

		// here we scale the model to make it into a cuboid
		model.top() = scale(model.top(), vec3(scale_x, scale_y, scale_z));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);
	}
	model.pop();
}

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	// output the controls for the application
	cout << "------------------ CONTROLS FOR APPLICATION------------------ " << endl;
	cout << "WASD and QE for viewing the object from different angles" << endl;
	cout << "Z and X to make the object smaller and larger" << endl;
	cout << "O to open and P to close the box and U to make the angel spin" << endl;
	cout << "1234567890 for light controls" << endl;
	cout << "Change the colour mode for the program" << endl;

	/* Load and create our monkey object*/
	monkey.load_obj("monkey_normals.obj");
	// Set all the colour vertices to blue
	// Note that this is a quick hack, it would be better to load and process the mtl file
	monkey.overrideColour(vec4(0.5f, 0, 1.0, 1.f));

	/* Set the object transformation controls to their initial values */
	speed = 0.05f;

	open_lid = false;

	// set the angle and the speed of the angel
	angel_speed = 0.f;
	angle_angle = 0.f;
	light_height = 0.2;

	x = 0.05f;
	y = 0;
	z = 0;
	vx = 0; vx = 0, vz = 0.f;
	light_x = 0; light_y = 0.2; light_z = 0;
	angle_x = angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 1; emitmode = 0;
	attenuationmode = 1; // Attenuation is on by default
	numlats = 40;		// Number of latitudes in our sphere
	numlongs = 40;		// Number of longitudes in our sphere

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("poslight.vert", "poslight.frag");
	}
	catch (exception &e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program, "model");
	colourmodeID = glGetUniformLocation(program, "colourmode");
	emitmodeID = glGetUniformLocation(program, "emitmode");
	attenuationmodeID = glGetUniformLocation(program, "attenuationmode");
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");
	lightposID = glGetUniformLocation(program, "lightpos");
	normalmatrixID = glGetUniformLocation(program, "normalmatrix");

	/* create our sphere and cube objects */
	aSphere.makeSphere(numlats, numlongs);
	aCube.makeCube(vec3(0.7,0.2,0.1));
	aCylinder.makeCylinder();
	aprism.makeprism(vec3(0.5,0.0,0.5));


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
	glUseProgram(program);

	// Define our model transformation in a stack and 
	// push the identity matrix onto the stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Define the normal matrix
	mat3 normalmatrix;

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, 2, 4), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// Apply rotations to the view position. This wil get appleid to the whole scene
	view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = rotate(view, -radians(vy), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = rotate(view, -radians(vz), vec3(0, 0, 1));

	// Define the light position and transform by the view matrix
	vec4 lightpos = view *  vec4(light_x, light_y, light_z, 1.0);

	// Send our projection and view uniforms to the currently bound shader
	// I do that here because they are the same for all objects
	glUniform1ui(colourmodeID, colourmode);
	glUniform1ui(attenuationmodeID, attenuationmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID, 1, value_ptr(lightpos));

	// Define the global model transformations (rotate and scale). Note, we're not modifying thel light source position
	model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis
	model.top() = rotate(model.top(), -radians(angle_x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model.top() = rotate(model.top(), -radians(angle_y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model.top() = rotate(model.top(), -radians(angle_z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	

	/* Draw a small sphere in the lightsource position to visually represent the light source */
	model.push(model.top());
	{
		light_rise_speed = speed / 60;

		// if the lid is opening then change the height of the light
		if (open_lid && light_y <= 1) light_y += light_rise_speed;
		else if(open_lid == false && light_y >= 0.2) light_y -= light_rise_speed *3.5;

		model.top() = translate(model.top(), vec3(light_x, light_y, light_z));
		model.top() = scale(model.top(), vec3(0.05f, 0.05f, 0.05f)); // make a small sphere
																	 // Recalculate the normal matrix and send the model and normal matrices to the vertex shader																							// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																						// Recalculate the normal matrix and send to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our lightposition sphere  with emit mode on*/
		emitmode = 1;
		glUniform1ui(emitmodeID, emitmode);
		aSphere.drawSphere(drawmode);
		emitmode = 0;
		glUniform1ui(emitmodeID, emitmode);
	}
	model.pop();


	// This block of code draws the cube
	model.push(model.top());
	{

		// here we scale the model to make it into a cuboid
		model.top() = scale(model.top(), vec3(1.f, 0.05f, 1.f));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);
	}
	model.pop();


	// This block of code draws side at the back of the angel
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(0.f, 0.125f, -0.25f));

		// here we scale the model to make it into a cuboid
		model.top() = scale(model.top(), vec3(1.f, 0.5f, 0.05f));
	
		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);
	}
	model.pop();

	// This block of code draws the side at the front of the angel
	model.push(model.top());
	{
		// if the lid is opening then change the angle of the door
		if (open_lid && lid_angle <= 90) lid_angle += speed;
		else if (lid_angle >= 0)lid_angle -= speed;


		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(0.f, 0.f, 0.25f));

		// rotate the model x degrees
		model.top() = rotate(model.top(), radians(lid_angle), vec3(1.f, 0.f, 0.f));

		// Move the lid to the right place for the box
		model.top() = translate(model.top(), vec3(0.f, 0.125f, 0.f));

		// here we scale the model to make it into a cuboid
		model.top() = scale(model.top(), vec3(1.f, 0.5f, 0.05f));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);
	}
	model.pop();


	// This block of code draws the side to the right of the angel
	model.push(model.top());
	{

		// if the lid is opening then change the angle of the door
		if (open_lid && lid_angle <= 90) lid_angle += speed;
		else if (lid_angle >= 0)lid_angle -= speed;


		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(0.25f, 0.f, 0.f));

		// rotate the model x degrees
		model.top() = rotate(model.top(), radians(-lid_angle), vec3(0.f, 0.f, 1.f));

		// Move the lid to the right place for the box
		model.top() = translate(model.top(), vec3(0.f, 0.125f, 0.f));

		// here we scale the model to make it into a cuboid
		model.top() = scale(model.top(), vec3(0.05f, 0.5f, 1.f));


		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);
	}
	model.pop();

	// This block of code draws the side on the left of the box
	model.push(model.top());
	{

		// if the lid is opening then change the angle of the door
		if (open_lid && lid_angle <= 90) lid_angle += speed;
		else if (lid_angle >= 0)lid_angle -= speed;


		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(-0.25f, 0.f, 0.f));

		// rotate the model x degrees
		model.top() = rotate(model.top(), radians(lid_angle), vec3(0.f, 0.f, 1.f));

		// Move the lid to the right place for the box
		model.top() = translate(model.top(), vec3(0.f, 0.125f, 0.f));

		// here we scale the model to make it into a cuboid
		model.top() = scale(model.top(), vec3(0.05f, 0.5f, 1.f));
	
		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);
	}
	model.pop();

	// This block of code draws the cylinder at the base of the rotation
	model.push(model.top());
	{

		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(0.f,0.02f,0.f));

		// here we scale the model to make it into a flat cylinder
		model.top() = scale(model.top(), vec3(0.125f, 0.05f, 0.125f));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cylinder*/
		aCylinder.drawCylinder(drawmode);
	}
	model.pop();

	// This block of code draws the monkey which is the angel inside the box
	model.push(model.top());
	{

		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(0.f, 0.1f, 0.f));

		// here we scale the model to make it into a cuboid
		model.top() = scale(model.top(), vec3(0.05f, 0.05f, 0.05f));

		angle_angle += angel_speed;

		// rotate it at the speed that it needs to go at
		model.top() = rotate(model.top(), radians(angle_angle), vec3(0.f, 1.f, 0.f));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		/* Draw our Blender Monkey object */
		monkey.drawObject(drawmode);
	}
	model.pop();

	// This block of code draws the prism which is the angel's hat inside the box
	model.push(model.top());
	{

		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(0.f, 0.15f, 0.f));

		model.top() = scale(model.top(), vec3(0.05f, 0.05f, 0.05f));

		angle_angle += angel_speed;

		// rotate it at the speed that it needs to go at
		model.top() = rotate(model.top(), radians(angle_angle), vec3(0.f, 1.f, 0.f));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our prism*/
		aprism.drawprism(drawmode);
	}
	model.pop();


	// This block of code draws the cube at the top of the box
	model.push(model.top());
	{

		// if the lid is opening then change the angle of the door
		if (open_lid && lid_angle <= 90) lid_angle += speed;
		else if(lid_angle >= 0)lid_angle -= speed;

		// is the log / exponent of the angle?
		// this does not happen linearly
		GLfloat z_coord = 0.125 * (lid_angle / (90 * 2));

		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(0.f, 0.25f, -0.25f));
		
		// rotate the model x degrees
		model.top() = rotate(model.top(), radians(-lid_angle), vec3(1.f, 0.f, 0.f));

		// Move the lid to the right place for the box
		model.top() = translate(model.top(), vec3(0.f, 0.f, 0.25f));

		// here we scale the model to make it into a cuboid
		model.top() = scale(model.top(), vec3(1.f, 0.05f, 1.f));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);
	}
	model.pop();
	
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

	if (key == 'W') angle_inc_x -= speed;
	if (key == 'S') angle_inc_x += speed;
	if (key == 'A') angle_inc_y -= speed;
	if (key == 'D') angle_inc_y += speed;
	if (key == 'Q') angle_inc_z -= speed;
	if (key == 'E') angle_inc_z += speed;
	if (key == 'Z') model_scale -= speed / 0.5f;
	if (key == 'X') model_scale += speed / 0.5f;
	//if (key == 'Z') x -= speed;
	//if (key == 'X') x += speed;
	//if (key == 'C') y -= speed;
	//if (key == 'V') y += speed;
	//if (key == 'B') z -= speed;
	//if (key == 'N') z += speed;
	// buttons for the movement of the box
	if (key == 'U') angel_speed += speed;
	if (key == 'O') open_lid = true;
	else if (key == 'P') open_lid = false;

	if (key == '1') light_x -= speed;
	if (key == '2') light_x += speed;
	if (key == '3') light_y -= speed;
	if (key == '4') light_y += speed;
	if (key == '5') light_z -= speed;
	if (key == '6') light_z += speed;
	if (key == '7') vx -= 1.f;
	if (key == '8') vx += 1.f;
	if (key == '9') vy -= 1.f;
	if (key == '0') vy += 1.f;
	// if (key == 'O') vz -= 1.f;
	// if (key == 'P') vz += 1.f;

	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}

	/* Turn attenuation on and off */
	if (key == '.' && action != GLFW_PRESS)
	{
		attenuationmode = !attenuationmode;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == ',' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}
}


/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Position light example");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	/* Output the OpenGL vendor and version */
	glw->DisplayVersion();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}

