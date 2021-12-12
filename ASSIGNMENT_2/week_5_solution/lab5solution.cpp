/*
 Lab5 Solution (Texturing)
 Creates a cube, a sphere and a quad with texture coordinates
 Includes rotation, scaling, translation, view and perspective transformations,
 Objects have normals,colours and texture coordinates defined
 for all vertices.
 Iain Martin November 2021
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
#include <stack>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

/* Include the image loader */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* Include the cube and sphere objects with texture coordinates */
#include "sphere_tex.h"
#include "cube_tex.h"
#include "quad_tex.h"
#include "points.h"

// Include our sphere and object loader classes
#include "tiny_loader.h"
#include "sphere_tex.h"

// include file to make terrain
#include "terrain_object.h"


GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */
GLuint emitmode;

GLuint fogmode;		/* Cycle between different fog equations */
GLfloat light_x, light_y, light_z;


GLuint program;		/* Identifier for the main shader prgoram */
GLuint points_program;		/* Identifier for the point_sprites shader prgoram */
GLuint obj_ldr_program;
GLuint terrain_program;

GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */


/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, scaler, z, y, tree_y, cube_y;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;	//Define the resolution of the sphere object

/* Uniforms*/
GLuint modelID, viewID, projectionID, colourmodeID; // uniforms for lighting shaders
GLuint points_modelID, points_viewID, points_projectionID, points_sizeID; // Uniforms for the points shaders
GLuint lightposID, normalmatrixID;
GLuint emitmodeID, fogmodeID;


/* Uniforms*/
GLuint terrain_modelID, terrain_viewID, terrain_projectionID, terrain_colourmodeID; // uniforms for lighting shaders


GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;

// Define texture ID value (identifier for a specific texture)
GLuint textureID1, textureID2, textureID3, point_textureID;

/* Point sprite object and adjustable parameters */
points* point_anim;
GLfloat speed;
GLfloat maxdist;
GLfloat point_size;		// Used to adjust point size in the vertex shader


// Quad object
GLuint quad_vbo;

Sphere sphere;
Cube cube(true);
Quad quad(true);

TinyObjLoader tiny_obj;		// This is an instance of our tiny object loader with texture
Sphere aSphere(false);		// Create our sphere with no texture coordinates because they aren't handled in the shaders for this example

/* Define textureID*/
GLuint texID;

// terrain objects
terrain_object* heightfield;
int octaves;
GLfloat perlin_scale, perlin_frequency;
GLfloat land_size;
GLfloat sealevel = 0;


using namespace std;
using namespace glm;


// Define fogmode strings to output to screen
string fog_mode_desc[] = { "off", "linear", "exp", "exp2" };


bool load_texture(const char* filename, GLuint& texID, bool bGenMipmaps)
{
	// Obtain an unused texture identifier. 
	glGenTextures(1, &texID);

	// local image parameters
	int width, height, nrChannels;

	/* load an image file using stb_image */
	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

	// check for an error during the load process
	if (data)
	{
		// Note: this is not a full check of all pixel format types, just the most common two!
		int pixel_format = 0;
		if (nrChannels == 3)
			pixel_format = GL_RGB;
		else
			pixel_format = GL_RGBA;

		// Bind the texture ID before the call to create the texture.
			// texID[i] will now be the identifier for this specific texture
		glBindTexture(GL_TEXTURE_2D, texID);

		// Create the texture, passing in the pointer to the loaded image pixel data
		glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);

		// Generate Mip Maps
		if (bGenMipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			// If mipmaps are not used then ensure that the min filter is defined
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
	}
	else
	{
		printf("stb_image  loading error: filename=%s", filename);
		return false;
	}
	stbi_image_free(data);
	return true;
}

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper* glw)
{
	/* Set the object transformation controls to their initial values */
	x = 0.05f;
	y = 0;
	z = 0;
	angle_x = angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	scaler = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 0;
	numlats = 100;		// Number of latitudes in our sphere
	numlongs = 100;		// Number of longitudes in our sphere
	emitmode = 0;
	fogmode = 0;
	light_x = 0; light_y = 0; light_z = 0;

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Create the heightfield object */
	octaves = 2;
	perlin_scale = 2.f;
	perlin_frequency = 1.f;
	land_size = 40.f;
	heightfield = new terrain_object(octaves, perlin_frequency, perlin_scale);
	heightfield->createTerrain(200, 200, land_size, land_size);
	//heightfield->setColour(vec3(1, 1, 1));
	//heightfield->setColourBasedOnHeight();
	heightfield->createObject();


	// Creater the sphere (params are num_lats and num_longs)
	aSphere.makeSphere(60, 60);

	/* create the sphere and cube objects */
	sphere.makeSphere(numlats, numlongs);
	cube.makeCube();
	quad.makeQuad();

	/* Load and build the vertex and fragment shaders:  */
	try
	{
		program = glw->LoadShader("..\\ASSIGNMENT_2\\week_5_solution\\lab5solution.vert", "..\\ASSIGNMENT_2\\week_5_solution\\lab5solution.frag");
		points_program = glw->LoadShader("..\\ASSIGNMENT_2\\week_5_solution\\point_sprites.vert", "..\\ASSIGNMENT_2\\week_5_solution\\point_sprites_analytic.frag");
		terrain_program = glw->LoadShader("..\\ASSIGNMENT_2\\week_5_solution\\terrain.vert", "..\\ASSIGNMENT_2\\week_5_solution\\terrain.frag");
//		points_program = glw->LoadShader("point_sprites.vert", "point_sprites.frag");
	}
	catch (exception& e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}


	/* Load and create our object*/
	tiny_obj.load_obj("..\\ASSIGNMENT_2\\week_5_solution\\tree.obj");

	load_texture("..\\ASSIGNMENT_2\\week_5_solution\\grass.jpg", texID, false);

	/* Define uniforms to send to main program shaders */
	modelID = glGetUniformLocation(program, "model");
	colourmodeID = glGetUniformLocation(program, "colourmode");
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");
	emitmodeID = glGetUniformLocation(program, "emitmode");
	lightposID = glGetUniformLocation(program, "lightpos");
	normalmatrixID = glGetUniformLocation(program, "normalmatrix");
	fogmodeID = glGetUniformLocation(program, "fogmode");

	/* Define uniforms to send to main program shaders */
	terrain_modelID = glGetUniformLocation( terrain_program, "model");
	terrain_colourmodeID = glGetUniformLocation(terrain_program, "colourmode");
	terrain_viewID = glGetUniformLocation(terrain_program, "view");
	terrain_projectionID = glGetUniformLocation(terrain_program, "projection");

	/* Define uniforms to send to point sprites program shaders */
	points_modelID = glGetUniformLocation(points_program, "model");
	points_sizeID = glGetUniformLocation(points_program, "size");
	points_viewID = glGetUniformLocation(points_program, "view");
	points_projectionID = glGetUniformLocation(points_program, "projection");

	// Place the present object on the terrain at its current start position
	cube_y = heightfield->heightAtPosition(1.f, 1.f);
	tree_y = heightfield->heightAtPosition(x, z);

	

	// Call our texture loader function to load two textures.
	// Note that our texture loader generates the texID and is passed as a var parameter
	// The third parameter is a boolean that with generater mipmaps if true

	/* load an image file using stb_image */
	const char* filename1 = "..\\ASSIGNMENT_2\\week_5_solution\\grass.jpg";
	const char* filename2 = "..\\ASSIGNMENT_2\\week_5_solution\\earth_no_clouds.jpg";
	const char* filename3 = "..\\ASSIGNMENT_2\\week_5_solution\\ground1.jpg";
	const char* point_texture_file = "..\\ASSIGNMENT_2\\week_5_solution\\star.png";


	// This will flip the image so that the texture coordinates defined in
	// the sphere, match the image orientation as loaded by stb_image
	stbi_set_flip_vertically_on_load(true);
	if (!load_texture(filename1, textureID1, true))
	{
		cout << "Fatal error loading texture: " << filename1 << endl;
		exit(0);
	}

	// Load second texture image
	if (!load_texture(filename2, textureID2, false))
	{
		cout << "Fatal error loading texture: " << filename2 << endl;
		exit(0);
	}

	// Load third texture image
	if (!load_texture(filename3, textureID3, true))
	{
		cout << "Fatal error loading texture: " << filename2 << endl;
		exit(0);
	}

	// Load point sprite texture image
	if (!load_texture(point_texture_file, point_textureID, true))
	{
		cout << "Fatal error loading texture: " << point_texture_file << endl;
		exit(0);
	}

	// This is the location of the texture object (TEXTURE0), i.e. tex1 will be the name
	// of the sampler in the fragment shader
	int loc = glGetUniformLocation(program, "tex1");
	if (loc >= 0) glUniform1i(loc, 0);

	// Define the texture uniform for the points shader program
	int loc_points = glGetUniformLocation(points_program, "tex1");
	if (loc_points >= 0) glUniform1i(loc_points, 0);

	// SET Texture MAG_FILTER to linear which will blur the texture if we
	// zoom too close in
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Define obejct and variables for the point sprites
	speed = 0.1f;
	maxdist = 1.f;
	point_anim = new points(5000, maxdist, speed);
	point_anim->create();
	point_size = 8;
	/* Define the Blending function */
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, 0, 4), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);


	// Define the model transformation stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Define the model transformations for both the cube and the sphere
	model.top() = scale(model.top(), vec3(scaler, scaler, scaler));//scale equally in all axis
	model.top() = rotate(model.top(), -radians(angle_x), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model.top() = rotate(model.top(), -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model.top() = rotate(model.top(), -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
	
																		 // Send our uniforms variables to the currently bound shader,

	// Draw our tree
	model.push(model.top());
	{
		// Now draw our particles
		/* switch to the terrain shader program current */
		glUseProgram(terrain_program);

		// Send our common uniforms variables to the currently bound shader,
		glUniform1ui(colourmodeID, colourmode);
		glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);

		glUniformMatrix4fv(modelID, 1, GL_FALSE, &model.top()[0][0]);

		// Enable gl_PointSize
		glEnable(GL_PROGRAM_POINT_SIZE);
		/* Enable Blending for the analytic point sprite */
		glEnable(GL_BLEND);

		// Draw our quad
		heightfield->drawObject(drawmode);
	}



	// Define the normal matrix
	mat3 normalmatrix = transpose(inverse(mat3(view * model.top())));

	// Define the light position and transform by the view matrix
	vec4 lightpos = view * vec4(light_x, light_y, light_z, 1.0);



	/* Make the main compiled shader program current */
	glUseProgram(program);

	// Send our common uniforms variables to the currently bound shader,
	glUniform1ui(colourmodeID, colourmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform1ui(emitmodeID, emitmode);
	glUniform4fv(lightposID, 1, value_ptr(lightpos));
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
	glUniform1ui(fogmodeID, fogmode);




	

	// Draw our tree
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(x, tree_y, z));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);


		/* Draw our object with texture */
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// draw the object
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &model.top()[0][0]);
		tiny_obj.drawObject(drawmode);
		glBindTexture(GL_TEXTURE_2D, 0);

	}
	model.pop();

	// Draw our sphere
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(-x - 0.5, 0, 0));
		model.top() = scale(model.top(), vec3(scaler / 3.f, scaler / 3.f, scaler / 3.f));//scale equally in all axis
	
		
		// Bind the map texture, no mipmap for this texture so change texture parameter as otherwsie will not work
		glBindTexture(GL_TEXTURE_2D, textureID2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Draw our sphere
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &model.top()[0][0]);
		sphere.drawSphere(drawmode);
	
	}
	model.pop();

	// Draw a cube object with a different texture
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(x, tree_y + 0.1f, z));	// +0.1 t0 place on the terrain
		model.top() = scale(model.top(), vec3(0.1f, 0.1f, 0.1f));//scale equally in all axis
		model.top() = rotate(model.top(), -radians(angle_x - 50.f), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
		model.top() = rotate(model.top(), -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
		model.top() = rotate(model.top(), -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

		glUniformMatrix4fv(modelID, 1, GL_FALSE, &model.top()[0][0]);

		// Bind the ground texture, change texture parameter to make use of mipmap
		glBindTexture(GL_TEXTURE_2D, textureID3);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// Draw our quad
		cube.drawCube(drawmode);
	}
	model.pop();





	// Now draw our particles
	/* switch to the point sprites shader program current */
	glUseProgram(points_program);

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(points_modelID, 1, GL_FALSE, &model.top()[0][0]);
	glUniform1f(points_sizeID, point_size);
	glUniformMatrix4fv(points_viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(points_projectionID, 1, GL_FALSE, &projection[0][0]);

	// Enable gl_PointSize
	glEnable(GL_PROGRAM_POINT_SIZE);
	/* Enable Blending for the analytic point sprite */
	glEnable(GL_BLEND);

	// Bind the ground texture, change texture parameter to make use of mipmap
	glBindTexture(GL_TEXTURE_2D, point_textureID);

	/* Modify our animation variables */
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;

	point_anim->draw();
	point_anim->animate();

	// Disable everything
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableVertexAttribArray(0);
	glDisable(GL_PROGRAM_POINT_SIZE);
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

	if (key == 'W') angle_inc_x -= 0.05f;
	if (key == 'S') angle_inc_x += 0.05f;
	if (key == 'A') angle_inc_y -= 0.05f;
	if (key == 'D') angle_inc_y += 0.05f;
	if (key == 'T') angle_inc_z -= 0.05f;
	if (key == 'Y') angle_inc_z += 0.05f;
	if (key == '-') scaler -= 0.02f;
	if (key == '=') scaler += 0.02f;
	if (key == 'Z') x -= 0.05f;
	if (key == 'X') x += 0.05f;
	if (key == 'C') y -= 0.05f;
	if (key == 'V') y += 0.05f;
	if (key == 'B') z -= 0.05f;
	if (key == 'N') z += 0.05f;

	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == 'N' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}

	/* Cycle between fogmodes */
	/* Switch between the vertex lighting shaders and the fragment shader */
	if (key == 'F' && action != GLFW_PRESS)
	{
		fogmode == 3 ? fogmode = 0 : fogmode++;
		cout << "Fogmode: " << fog_mode_desc[fogmode] << endl;
	}

	/* Point sprite animation parameters */
	if (key == ',') point_size -= 1.f;
	if (key == '.') point_size += 1.f;
	if (key == 'O') speed -= 0.1f;
	if (key == 'P') speed += 0.1f;
	if (key == 'L') maxdist -= 0.1f;
	if (key == ';') maxdist += 0.1f;

	point_anim->updateParams(maxdist, speed);

}


/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Lab5 Solution: Textured cube and sphere");

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


