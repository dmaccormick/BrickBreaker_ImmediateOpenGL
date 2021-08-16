// Block.h
// - Pure Virtual Class
// - Will have two children -> single hit blocks and double hit blocks
// - Child of game object class
// - Made so that we can use polymorphism in main block array
//
//SingleHitBlock
// - Dies in one hit
//
//DoubleHitBlocks
// - Dies in two hits
// - Becomes a singleHitBlock after the first hit

#ifndef BLOCK_H
#define BLOCK_H

#include "GameObject.h"

class Block : public GameObject
{
public:
	Block() : GameObject() {}
	~Block() {}

	virtual bool collisionDeath() = 0; //Return bool represents if object is dead or not (true = dead)
	bool singleHitType;
};

class SingleHitBlock : public Block
{
public:
	SingleHitBlock() : Block() { colour = glm::vec3(1.0f, 0.0f, 0.0f); singleHitType = true; }
	~SingleHitBlock() {}

	bool collisionDeath() { return true; }; //Immediately returns true because the single hit blocks immediately die
};

class DoubleHitBlock : public Block
{
public:
	DoubleHitBlock() : Block() { colour = glm::vec3(0.0f, 0.0f, 1.0f); singleHitType = false; }
	~DoubleHitBlock() {}

	//Returns if the block died or not. If still blue, transitions to red type here
	bool collisionDeath()
	{
		healthRemaining--;

		if (healthRemaining == 0)
			return true;
		else
		{
			colour = glm::vec3(1.0f, 0.0f, 0.0f);
			singleHitType = true;
			return false;
		}
	}

private:
	int healthRemaining = 2;
};

#endif