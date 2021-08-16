//Particle class
//Has the following
// - position
// - colour
// - lifetime
// - update function

#ifndef PARTICLE_H
#define PARTICLE_H

#include "GLUT\glut.h"
#include "GLM\glm.hpp"

struct Particle
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 acceleration = glm::vec3(0.0f, -9.8f, 0.0f);
	glm::vec4 colour = glm::vec4(1.0f);
	float maxLifeTime = 1.0f;
	float totalLifeTime = 0.0f;

	bool update(float deltaT) //Returns if the particle is still alive or not
	{
		totalLifeTime += deltaT;

		if (totalLifeTime < maxLifeTime)
		{
			//Updates the position using euler intergration
			velocity += (acceleration * deltaT);
			position += (velocity * deltaT);
			
			float transparency = totalLifeTime / maxLifeTime;

			if (transparency > 0.5f)
			{
				//Fades out as the particle dies
				colour.w = 1.0f - (totalLifeTime / maxLifeTime);
			}
			
			return true;
		}
		else
			return false;
	}
};

#endif
