// Daniel MacCormick - 100580519
// GameObject.h
// - Class that represents a object in game
// - Bare minimum must have
// -- a renderable (a sprite, a 3d obj, a wavefront obj)
// -- A position in gamespace (x,y,z)
// - additional properties may be
// -- velocity
// -- acceleration
// -- rotations (x, y, z)
// -- texture
#pragma once

#include <GLM\glm.hpp>
#include "cube.h"
#include <iostream>

class GameObject
{
public:
	GameObject();
	GameObject(glm::vec3 pos, glm::vec3 rot, glm::vec3 scl, glm::vec3 vel, glm::vec3 accel);
	~GameObject() {}

	void draw() const;
	void update(float deltaT);

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	glm::vec3 velocity;
	glm::vec3 acceleration;

	glm::vec3 colour;

	GLuint textureNumber = NULL;

	bool active; //If the gameobject is to be updated / drawn or not (ie: is it 'alive')
};

GameObject::GameObject()
{
	position = glm::vec3(0.0f);
	rotation = glm::vec3(0.0f);
	scale = glm::vec3(1.0f);
	velocity = glm::vec3(0.0f);
	acceleration = glm::vec3(0.0f);
	colour = glm::vec3(1.0f);
}

GameObject::GameObject(glm::vec3 pos, glm::vec3 rot, glm::vec3 scl, glm::vec3 vel, glm::vec3 accel)
{
	position = pos;
	rotation = rot;
	scale = scl;
	velocity = vel;
	acceleration = accel;
	colour = glm::vec3(1.0f);
}

void GameObject::update(float deltaT)
{
	if (active)
	{
		velocity = velocity + (acceleration * deltaT);
		position = position + (velocity*deltaT);
	}
}

void GameObject::draw() const
{
	if (active)
	{
		glPushMatrix();

		glTranslatef(position.x, position.y, position.z);																																																																																																																																																		
		glRotatef(rotation.x, 1.0f, 0.0f, 0.0f);
		glRotatef(rotation.y, 0.0f, 1.0f, 0.0f);
		glRotatef(rotation.z, 0.0f, 0.0f, 1.0f);
		glScalef(scale.x, scale.y, scale.z);

		Cube::draw();

		glPopMatrix();
	}
}

bool checkCollision(GameObject& a, GameObject& b)
{
	glm::vec3 scaleA = a.scale;
	glm::vec3 scaleB = b.scale;

	float thisMinX = a.position.x - scaleA.x; // position - width/2
	float thisMaxX = a.position.x + scaleA.x;
	float thisMinY = a.position.y - scaleA.y;
	float thisMaxY = a.position.y + scaleA.y;
	float thisMinZ = a.position.z - scaleA.z;
	float thisMaxZ = a.position.z + scaleA.z;

	float otherMinX = b.position.x - scaleB.x; // position - width/2
	float otherMaxX = b.position.x + scaleB.x;
	float otherMinY = b.position.y - scaleB.y;
	float otherMaxY = b.position.y + scaleB.y;
	float otherMinZ = b.position.z - scaleB.z;
	float otherMaxZ = b.position.z + scaleB.z;

	// check overlap X
	if ((thisMinX >= otherMinX && thisMinX <= otherMaxX) ||
		(thisMaxX >= otherMinX && thisMaxX <= otherMaxX))
	{
		// check overlap Y
		if ((thisMinY >= otherMinY && thisMinY <= otherMaxY) ||
			(thisMaxY >= otherMinY && thisMaxY <= otherMaxY))
		{
			// check overlap Z
			if ((thisMinZ >= otherMinZ && thisMinZ <= otherMaxZ) ||
				(thisMaxZ >= otherMinZ && thisMaxZ <= otherMaxZ))
			{
				return true;
			}
		}
	}
	return false;
}

//Spherical collision
bool checkRadialCollision(GameObject& a, GameObject &b, float aRadius, float bRadius)
{
	float minimumSeparation = aRadius + bRadius;

	float dist = glm::distance(a.position, b.position);

	if (dist <= minimumSeparation)
		return true;
	else
		return false;
}

//Function to determine collision response
void collisionResponse(GameObject& a, GameObject& b)
{
	//Calculate first object's speed
	float speed = glm::length(a.velocity);

	//The normal of the collision, what the incident and reflected rays will be relative to
	glm::vec3 collisionNormal = glm::normalize(b.position - a.position);

	//Roughly pushes the objects out of eachother to prevent them from colliding next frame too
	a.position -= (glm::normalize(collisionNormal) * 0.5f);

	//Accounting for fringe case where object is stationary
	if (speed != 0.0f)
	{
		//Account for fringe case of zeroed direction
		if (glm::length(glm::normalize(a.velocity) - collisionNormal) == 0.0f)
		{
			a.velocity = collisionNormal * -speed;
		}
		else
		{
			//Maintains speed after collision but in the correct direction
			a.velocity = glm::normalize(glm::normalize(a.velocity) - collisionNormal) * speed;
		}
	}
	else
	{
		//Common to just leave this empty and do nothing (basically means this object is simply static)

		//However, we could just repeat the collision response as above but backwards (take object b's properties and apply them to a so a moves too)
		float speed = glm::length(b.velocity);

		if (speed != 0.0f)
		{
			//Account for fringe case of zeroed direction
			if (glm::length(glm::normalize(b.velocity) + collisionNormal) == 0.0f) //Use b's velocity and ADD collision normal, not subtract
			{
				a.velocity = collisionNormal * -speed;
			}
			else
			{
				//Maintains speed after collision but in the correct direction
				a.velocity = (glm::normalize(b.velocity) - collisionNormal) * speed * 0.5f; //Transfer half of b's velocity to a
			}
		}
	}
}