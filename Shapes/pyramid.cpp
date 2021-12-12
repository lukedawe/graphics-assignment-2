/* pyramid.h
 Example class to create a generic pyramid object
 Iain Martin October 2018
*/

#include "pyramid.h"

/* I don't like using namespaces in header files but have less issues with them in
seperate cpp files */
using namespace std;

/* Define the vertex attributes for vertex positions and normals.
Make these match your application and vertex shader
You might also want to add colours and texture coordinates */
pyramid::pyramid()
{
	attribute_v_coord = 0;
	attribute_v_colours = 1;
	attribute_v_normal = 2;
	numvertices = 12;
}


pyramid::~pyramid()
{
}


/* Make a pyramid from hard-coded vertex positions and normals  */
void pyramid::makepyramid()
{
	/* Define vertices for a pyramid in 12 triangles */
	GLfloat vertexPositions[] =
	{

		// front of the prism
		-0.25f, 0.0f, 0.0f,
		 0.25f, 0.0f, 0.0f,
		 0.0f, 0.25f, 0.0f,

		 // underside of the prism
		 -0.25f, 0.0f, 0.0f,
		 0.25f, 0.0f, 0.0f,
		 -0.25f, 0.0f, 0.25f,

		 -0.25f, 0.0f, 0.25f,
		 0.25f, 0.0f, 0.25f,
		 0.25f, 0.0f, 0.0f,

		 // 'right' side of the prism
		 0.0f, 0.25f, 0.0f,
		 0.25f, 0.0f, 0.0f,
		 0.0f, 0.25f, 0.25f,

		 0.0f, 0.25f, 0.25f,
		 0.25f, 0.0f, 0.25f,
		 0.25f, 0.0f, 0.0f,

		 // 'left side of the prism
		 0.0f, 0.25f, 0.0f,
		 -0.25f, 0.0f, 0.0f,
		 0.0f, 0.25f, 0.25f,

		 0.0f, 0.25f, 0.25f,
		 -0.25f, 0.0f, 0.25f,
		 -0.25f, 0.0f, 0.0f,

		 // back of the prism
		 -0.25f, 0.0f, 0.25f,
		 0.25f, 0.0f, 0.25f,
		 0.0f, 0.25f, 0.25f,



	};

	/* Manually specified colours for our pyramid */
	float vertexColours[] = {
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

	};

	/* Manually specified normals for our pyramid */
	GLfloat normals[] =
	{
		// we have 24 vertices



		-1.f, 0.f, 0.f, 
		-1.f, 0.f, 0.f,
		-1.f, 0.f, 0.f,

		0, 0, -1.f, 
		0, 0, -1.f,		
		1.f, 0, 0, 
		1.f, 0, 0, 
		1.f, 0, 0,
		0, 0, 1.f, 
		0, 0, 1.f, 
		0, 0, 1.f,
		-1.f, 0, 0, 
		-1.f, 0, 0, 
		-1.f, 0, 0,
		0, -1.f, 0, 
		0, -1.f, 0,
		0, -1.f, 0,
		0, 1.f, 0, 
		0, 1.f, 0, 
		0, 1.f, 0,

	};

	/* Create the vertex buffer for the pyramid */
	glGenBuffers(1, &positionBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Create the colours buffer for the pyramid */
	glGenBuffers(1, &colourObject);
	glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColours), vertexColours, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Create the normals  buffer for the pyramid */
	glGenBuffers(1, &normalsBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


/* Draw the pyramid by bining the VBOs and drawing triangles */
void pyramid::drawpyramid(int drawmode)
{
	/* Bind pyramid vertices. Note that this is in attribute index attribute_v_coord */
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glEnableVertexAttribArray(attribute_v_coord);
	glVertexAttribPointer(attribute_v_coord, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind pyramid colours. Note that this is in attribute index attribute_v_colours */
	glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	glEnableVertexAttribArray(attribute_v_colours);
	glVertexAttribPointer(attribute_v_colours, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind pyramid normals. Note that this is in attribute index attribute_v_normal */
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
	else // Draw the pyramid in triangles
	{
		glDrawArrays(GL_TRIANGLES, 0, numvertices * 3);
	}
}