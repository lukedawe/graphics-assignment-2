/** Basic Point class to use in an example particle animation 
 Iain Martin
 November 2018
 */

#include "points.h"
#include "glm/gtc/random.hpp"
#include <random>

/* Constructor, set initial parameters*/
points::points(GLuint number, GLfloat dist, GLfloat sp)
{
	numpoints = number;
	maxdist = dist;
	speed = sp;
}


points::~points()
{
	delete [] colours;
	delete[] vertices;
}

void points::updateParams(GLfloat dist, GLfloat sp)
{
	maxdist = dist;
	speed = sp;
}


void  points::create()
{
	vertices = new glm::vec3[numpoints];
	colours = new glm::vec3[numpoints];
	velocity = new glm::vec3[numpoints];

	/* Define random position and velocity */
	for (int i = 0; i < numpoints; i++)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> disxz(-20.0, 20.0);
		std::uniform_real_distribution<> disy(-20.0, 20.0);
		std::uniform_real_distribution<> disv(-0.005, -0.00025);

		vertices[i] = glm::vec3(disxz(gen), disy(gen), disxz(gen));
		colours[i] = glm::vec3(1.f, 1.f, 1.f);
		velocity[i] = glm::vec3(0.f, disv(gen), 0.f);
	}

	/* Create the vertex buffer object */
	/* and the vertex buffer positions */
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, numpoints * sizeof(glm::vec3), vertices, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &colour_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, colour_buffer);
	glBufferData(GL_ARRAY_BUFFER, numpoints * sizeof(glm::vec3), colours, GL_STATIC_DRAW);
}


void points::draw()
{
	/* Bind  vertices. Note that this is in attribute index 0 */
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind cube colours. Note that this is in attribute index 1 */
	glBindBuffer(GL_ARRAY_BUFFER, colour_buffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Draw our points*/
	glDrawArrays(GL_POINTS, 0, numpoints);
}


void points::animate()
{

	for (int i = 0; i < numpoints; i++)
	{
		// Add velocity to the vertices 
		vertices[i] += velocity[i];

		// Calculate distance to the origin
		GLfloat dist = vertices[i].y;

		// If we are near the origin then we introduce a new random direction
		if (dist < 0.01f) vertices[i].y = 20.f;

		// If we are too far away then change direction back to the origin
		//if (dist > 0.01f) velocity[i].y = -vertices[i].y / 500.f * speed;
		//if (dist > 0.01f) vertices[i].y = vertices[i].y - 0.1f;
	}


	// Update the vertex buffer data
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, numpoints * sizeof(glm::vec3), vertices, GL_DYNAMIC_DRAW);
}



