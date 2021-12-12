/* prism.h
 Create a prism 
 Luke Dawe 2021
*/

#include "prism.h"
#include <math.h>

/* I don't like using namespaces in header files but have less issues with them in
seperate cpp files */
using namespace std;

/* Define the vertex attributes for vertex positions and normals.
Make these match your application and vertex shader
You might also want to add colours and texture coordinates */
prism::prism()
{
	attribute_v_coord = 0;
	attribute_v_colours = 1;
	attribute_v_normal = 2;
	numvertices = 12;
}


prism::~prism()
{
}


/* Make a prism from hard-coded vertex positions and normals  */
void prism::makeprism(glm::vec3 colours)
{
	/* Define vertices for a prism in 12 triangles */
	GLfloat vertexPositions[] =
	{

		// front of the prism
		-0.25f, -0.25f, -0.25f,
		 0.25f, -0.25f, -0.25f,
		 0.0f, 0.25f, -0.25f,

		 // underside of the prism
		 -0.25f, -0.25f, -0.25f,
		 0.25f, -0.25f, -0.25f,
		 -0.25f, -0.25f, 0.25f,

		 -0.25f, -0.25f, 0.25f,
		 0.25f, -0.25f, 0.25f,
		 0.25f, -0.25f, -0.25f,

		 // 'right' side of the prism
		 0.0f, 0.25f, -0.25f,
		 0.25f, -0.25f, -0.25f,
		 0.0f, 0.25f, 0.25f,

		 0.0f, 0.25f, 0.25f,
		 0.25f, -0.25f, 0.25f,
		 0.25f, -0.25f, -0.25f,

		 // 'left side of the prism
		 0.0f, 0.25f, -0.25f,
		 -0.25f, -0.25f, -0.25f,
		 0.0f, 0.25f, 0.25f,

		 0.0f, 0.25f, 0.25f,
		 -0.25f, -0.25f, 0.25f,
		 -0.25f, -0.25f, -0.25f,

		 // back of the prism
		 -0.25f, -0.25f, 0.25f,
		 0.25f, -0.25f, 0.25f,
		 0.0f, 0.25f, 0.25f,

	};

	/* Manually specified colours for our prism */
	float vertexColours[] = {

		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,

		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,

		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,

		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,

		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,

		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,

		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,

		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,
		colours.x, colours.y, colours.z, 1.0f,

	};

	/* Manually specified normals for our prism */
	GLfloat normals[] =
	{
		// we have 24 vertices

		// front
		0.f, 0.f, -1.f,
		0.f, 0.f, -1.f,
		0.f, 0.f, -1.f,

		// underside
		0.f, -1.f, 0.f,
		0.f, -1.f, 0.f,
		0.f, -1.f, 0.f,

		0.f, -1.f, 0.f,
		0.f, -1.f, 0.f,
		0.f, -1.f, 0.f,

		// right side
		sqrt(2) / 2, sqrt(2) / 2,0.f,
		sqrt(2) / 2, sqrt(2) / 2,0.f,
		sqrt(2) / 2, sqrt(2) / 2,0.f,

		sqrt(2) / 2, sqrt(2) / 2,0.f,
		sqrt(2) / 2, sqrt(2) / 2,0.f,
		sqrt(2) / 2, sqrt(2) / 2,0.f,

		// left side
		-sqrt(2) / 2, sqrt(2) / 2,0.f,
		-sqrt(2) / 2, sqrt(2) / 2,0.f,
		-sqrt(2) / 2, sqrt(2) / 2,0.f,

		-sqrt(2) / 2, sqrt(2) / 2,0.f,
		-sqrt(2) / 2, sqrt(2) / 2,0.f,
		-sqrt(2) / 2, sqrt(2) / 2,0.f,

		// back
		0.f, 0.f, 1.f,
		0.f, 0.f, 1.f,
		0.f, 0.f, 1.f,

	};

	/* Create the vertex buffer for the prism */
	glGenBuffers(1, &positionBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Create the colours buffer for the prism */
	glGenBuffers(1, &colourObject);
	glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColours), vertexColours, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Create the normals  buffer for the prism */
	glGenBuffers(1, &normalsBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


/* Draw the prism by bining the VBOs and drawing triangles */
void prism::drawprism(int drawmode)
{
	/* Bind prism vertices. Note that this is in attribute index attribute_v_coord */
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glEnableVertexAttribArray(attribute_v_coord);
	glVertexAttribPointer(attribute_v_coord, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind prism colours. Note that this is in attribute index attribute_v_colours */
	glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	glEnableVertexAttribArray(attribute_v_colours);
	glVertexAttribPointer(attribute_v_colours, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind prism normals. Note that this is in attribute index attribute_v_normal */
	glEnableVertexAttribArray(attribute_v_normal);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBufferObject);
	glVertexAttribPointer(attribute_v_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glPointSize(3.f);

	// Switch between filled and wireframe modes
	if (drawmode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Draw points
	if (drawmode == 2)
	{
		glDrawArrays(GL_POINTS, 0, numvertices * 3);
	}
	else // Draw the prism in triangles
	{
		glDrawArrays(GL_TRIANGLES, 0, numvertices * 3);
	}
}