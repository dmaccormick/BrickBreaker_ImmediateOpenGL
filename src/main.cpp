// Brickbreaker / Arkanoid clone
// main.cpp 

// Core Libraries
#include <iostream>
#include <string>
#include <math.h>
#include <vector>

// 3rd Party Libraries
#include <GLUT\glut.h>
#include <IL\ilut.h>
#include <GLM\glm.hpp>
#include <GLM\gtx\rotate_vector.hpp>

// user headers
#include "loadObject.h"
#include "cube.h"
#include "InputManager.h"
#include "GameObject.h"
#include "HelperFunctions.h"
#include "Block.h"
#include "Particle.h"

// Defines and Core variables
#define FRAMES_PER_SECOND 60
const int FRAME_DELAY = 1000 / FRAMES_PER_SECOND; // Miliseconds per frame

int windowWidth = 800;
int windowHeight = 800;

int mousepositionX;
int mousepositionY;

// ----- Camera ----- //
glm::vec3 cameraPosition;
glm::vec3 forwardVector;
glm::vec3 rightVector;
float movementScalar;

// ----- Textures ----- //
std::vector<GLuint> textureHandles;

// ----- GameObjects ----- //
GameObject paddle;
GameObject ball;
std::vector<GameObject> walls;
const int NUM_BLOCKS = 35;
std::vector<Block*> blocks;
int lastBlockHit = -1;
float timeSinceLastBlockHit = 0.0f;

// ----- GameVariables ----- //
int numBallsRemaining = 3;
int score = 0; //1 point for every block
int gameStage = 0; //0 is gameplay, 1 is end but victory, 2 is end but game over

// ----- Visual Effects ----- //
// ---- Ball Trail
std::vector<glm::vec3> ballTrail;
const int trailLength = 50; //How many positions are stored for the trail ie: how long it is
glm::vec3 trailStartColour = glm::vec3(0.95f, 0.21f, 0.13f); //colour of the square at the start of the particle
glm::vec3 trailEndColour = glm::vec3(0.92f, 0.88f, 0.88f); //colour of the square at the end of the particle

// ---- Particles
std::vector<Particle> particles; //global list of particles that all particles get added to

// ---- Victory fireworks ---- //
float timeSinceLastFireworks = 0.0f;

// ----- A few conversions to know ----- //
float degToRad = 3.14159f / 180.0f;
float radToDeg = 180.0f / 3.14159f;

//Spawns the end fireworks, very similar to below where brick particles are spawned
void spawnVictoryFireworks()
{
	//Randomizes effect position to be somewhere in the top third of the screen so the effects have space to fall
	glm::vec3 spawnPosition = glm::vec3(HelperFunctions::randomFloat(-12.0f, 12.0f), HelperFunctions::randomFloat(4.0f, 12.0f), 0.0f);

	int spawnNumber = HelperFunctions::randomFloat(35, 50); //Spawns between 25 and 50 firework pieces (converted to an int)

	glm::vec4 colourA = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 colourB = glm::vec4(0.0f, 0.5f, 1.0f, 1.0f);

	//Creates and adds the particles to the list
	for (int i = 0; i < spawnNumber; i++)
	{
		Particle p;

		float t = HelperFunctions::randomFloat(0.0f, 1.0f);
		p.colour = HelperFunctions::LERP(colourA, colourB, t);

		p.position = spawnPosition + HelperFunctions::randomVec3(-1.0f, 1.0f);
		p.position.z = 0.0f;

		p.velocity = HelperFunctions::randomVec3(-10.0f, 10.0f, 5.0f);
		p.velocity.z = 0.0f;

		p.maxLifeTime = 3.0f;

		particles.push_back(p); //All particles share the same vector since they all behave the same
	}
}

//Creates block particles given that block's position as well what colour they should be
void spawnBlockParticles(glm::vec3 position, bool singleHitType)
{
	//Spawns 10 particles at the given block position if it is a red block (destroying block creates more particles than transition from blue to red)
	if (singleHitType)
	{
		for (int i = 0; i < 10; i++)
		{
			Particle p;

			p.colour = glm::vec4(0.93f, 0.09f, 0.22f, 1.0f); //Red block colour

			p.position = position + HelperFunctions::randomVec3(-1.0f, 1.0f);
			p.position.z = 0.0f; //Zeroes out z-value since randomVec3 returns a random number there too

			p.velocity = HelperFunctions::randomVec3(-5.0f, 5.0f);
			p.velocity.z = 0.0f;

			particles.push_back(p);
		}
	}
	else //Only spawns 5 particles if blue since the block isn't actually being destroyed and only being transitioned from blue to red
	{
		for (int i = 0; i < 5; i++)
		{
			Particle p;

			p.colour = glm::vec4(0.07f, 0.33f, 0.84f, 1.0f); //Blue block colour

			p.position = position + HelperFunctions::randomVec3(-1.0f, 1.0f);
			p.position.z = 0.0f; //Zeroes out z-value since randomVec3 returns a random number there too

			p.velocity = HelperFunctions::randomVec3(-5.0f, 5.0f);
			p.velocity.z = 0.0f;

			particles.push_back(p);
		}
	}
	
}

//Draws and updates the block particles
void updateBlockParticles(float deltaT)
{
	glBindTexture(GL_TEXTURE_2D, 0);

	for (unsigned int i = 0; i < particles.size(); i++)
	{
		//Updates the particle and checks if it is dead...if it is, erases it from the vector
		if (!particles[i].update(deltaT))
			particles.erase(particles.begin() + i);
	}
}

//Draws the ball trail as a series of thinner and differently coloured points
void updateBallTrail()
{
	//Diables GL_BLEND so the trail doesn't become transparent (doesn't look as good)
	glDisable(GL_BLEND);

	//Deletes the oldest saved position for the trail and then inserts the newest value at the front, effectively shuffling all the values back
	ballTrail.pop_back();
	ballTrail.insert(ballTrail.begin(), ball.position);
	
	//Loops through each of the trail points, randomly sizes it, colours it based on the distance along the trail, and sets the transparency...skips first 3 so trail appears behind ball, not directly on top of ball
	for (unsigned int i = 3; i < ballTrail.size(); i++)
	{
		//The percentage distance along the trail
		float t = (float(i) / float(ballTrail.size()));

		//LERP's the trail colour from the start colour to the end colour depending on the t value calculated above
		glm::vec3 trailColour = HelperFunctions::LERP(trailStartColour, trailEndColour, t);

		//Calculates the size of the point by starting with a random size between a min and max and then scaling that by the t value above
		float basePointSize = HelperFunctions::randomFloat(8.0f, 25.0f);
		float pointSize = (1 - t) * basePointSize;

		//Sets the opacity of the point so that the first point is opaque and the last point is basically transparent
		float pointOpacity = (1 - t);
		
		//Draws the point
		glColor4f(trailColour.x, trailColour.y, trailColour.z, 1.0f);
		glPointSize(pointSize);

		glBegin(GL_POINTS);
			glVertex3f(ballTrail[i].x, ballTrail[i].y, -5);
		glEnd();

		glPointSize(1.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	//Re-enables blending so transparency works
	glEnable(GL_BLEND);
}

//Draws the score display and the lives display
void drawHUD()
{
	//Makes a new UI viewport
	glViewport(0, 0, windowWidth, windowHeight);

	// ----- Draw Score ----- //
	int textureOffset = 8; //Number of textures in the texture handle vector before the score textures
	int scoreTensColoumn = score / 10;
	int scoreOnesColoumn = score % 10;

	//Resets the colour so the textures don't get tinted
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	//Draws tens coloumns
	glBindTexture(GL_TEXTURE_2D, textureHandles[textureOffset + scoreTensColoumn]);

	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 0.0f); glVertex2f(-2.0f, -9.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(0.0f,	-9.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(0.0f,	-7.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(-2.0f,	-7.0f);
	}
	glEnd();

	//Draws ones coloumns
	glBindTexture(GL_TEXTURE_2D, textureHandles[textureOffset + scoreOnesColoumn]);

	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, -9.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(2.0f, -9.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(2.0f, -7.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, -7.0f);
	}
	glEnd();

	// ---- Draw Remaining Lives Underneath Paddle ----- //
	glBindTexture(GL_TEXTURE_2D, textureHandles[4]);

	//Draws first life indicator
	if (numBallsRemaining > 0)
	{
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.75f, -14.5f);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(-0.75f, -14.5f);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(-0.75f, -13.5f);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.75f, -13.5f);
		}
		glEnd();
	}

	//Draws second life indicator
	if (numBallsRemaining > 1)
	{
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0f, 0.0f); glVertex2f(-0.5f, -14.5f);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(0.5f, -14.5f);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(0.5f, -13.5f);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(-0.5f, -13.5f);
		}
		glEnd();
	}

	//Draws second life indicator
	if (numBallsRemaining > 2)
	{
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0f, 0.0f); glVertex2f(0.75f, -14.5f);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(1.75f, -14.5f);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(1.75f, -13.5f);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(0.75f, -13.5f);
		}
		glEnd();
	}

	// ----- Draw end text, either victory or game over, depending on result ----- //
	if (gameStage != 0)
	{
		if (gameStage == 1) //If the player won
			glBindTexture(GL_TEXTURE_2D, textureHandles[7]);
		else if (gameStage == 2) //If the player lost
			glBindTexture(GL_TEXTURE_2D, textureHandles[6]);

		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0f, 0.0f); glVertex2f(-12.0f, -6.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(12.0f, -6.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(12.0f, 6.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(-12.0f, 6.0f);
		}
		glEnd();
	}
}

// separate, cleaner, draw function
void drawObjects()
{
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	//--- Draw the paddle ---//
	glBindTexture(GL_TEXTURE_2D, textureHandles[3]);
	paddle.draw();

	//--- Draw the walls---//
	glBindTexture(GL_TEXTURE_2D, textureHandles[5]);
	for (unsigned int i = 0; i < walls.size(); i++)
		walls[i].draw();

	//--- Draw the blocks ---//
	for (unsigned int i = 0; i < blocks.size(); i++)
	{
		if (blocks[i]->singleHitType)
			glBindTexture(GL_TEXTURE_2D, textureHandles[1]);
		else
			glBindTexture(GL_TEXTURE_2D, textureHandles[2]);

		blocks[i]->draw();
	}

	//--- Draw the ball and its trail---//
	glBindTexture(GL_TEXTURE_2D, textureHandles[4]);
	ball.draw();
	glBindTexture(GL_TEXTURE_2D, 0);
	updateBallTrail(); //Draws trail behind ball

	//--- Draw the block particles ---//
	for (unsigned int i = 0; i < particles.size(); i++)
	{
		Particle p = particles[i];
		glColor4f(p.colour.x, p.colour.y, p.colour.z, p.colour.w);

		glPointSize(10.0f);
		glBegin(GL_POINTS);
			glVertex3f(p.position.x, p.position.y, 0);
		glEnd();
		glPointSize(1.0f);
	}
	
	//---Draw the hud elements---//
	drawHUD();
}

void init()
{
	// ----- Seed random number generation ----- //
	srand(time(0));

	// ----- Init OpenGL States ----- //
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH); //GL_FLAT

	glEnable(GL_TEXTURE_2D);
	ilInit();
	iluInit();
	ilutRenderer(ILUT_OPENGL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// ----- Init Camera ----- //
	cameraPosition = glm::vec3(0.0f, 0.0f, 50.0f);
	forwardVector = glm::vec3(0.0f, 0.0f, -1.0f);
	rightVector = glm::vec3(1.0f, 0.0f, 0.0f);
	movementScalar = 0.1f;

	// ----- Init objects ----- //
	//Init paddle
	paddle = GameObject(glm::vec3(0.0f, -12.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(3.0f, 0.25f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f));
	paddle.colour = glm::vec3(1.0f, 0.0f, 0.0f);

	//Init ball
	glm::vec3 ballStartVelocity = HelperFunctions::randomVec3(-8.5f, 8.5f, 5.5f); //Randomizes velocity with feather of 5.5f
	ballStartVelocity.z = 0.0f; //Ensures Z velocity is 0 so the ball doesn't come out of the screen
	ball = GameObject(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.5f), ballStartVelocity, glm::vec3(0.0f));
	ball.colour = glm::vec3(0.5, 0.5, 0.2);
	
	//Init walls
	walls.push_back(GameObject(glm::vec3(-14.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 90.0f), glm::vec3(15.0f, 0.5f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f))); //Left wall
	walls.push_back(GameObject(glm::vec3(14.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 90.0f), glm::vec3(15.0f, 0.5f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f))); //Right wall
	walls.push_back(GameObject(glm::vec3(0.0f, 14.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(14.0f, 0.5f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f))); //Top wall

	for (unsigned int i = 0; i < walls.size(); i++)
		walls[i].colour = glm::vec3(0.6f, 0.6f, 0.6f);

	//Init blocks
	int xIndex = 0, yIndex = 0; //The block index...7 across the screen with 5 rows
	float startPosX = -12.0f, startPosY = 13.0f; //The position of the top left block, all other blocks are based off this
	
	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		//Polymorphic block pointer which will be added to the vector after it is set up
		Block* newBlock;

		//Randomly decides if the block is going to be a single hit block or a double hit block
		if (HelperFunctions::randomFloat(0.0f, 1.0f) > 0.5f)
			newBlock = new SingleHitBlock();
		else
			newBlock = new DoubleHitBlock();

		//Initializes the new block's properties
		newBlock->scale.x = 2.0f;

		glm::vec3 newBlockPosition;
		newBlockPosition.x = startPosX + (xIndex * 2 * newBlock->scale.x);
		newBlockPosition.y = startPosY - (yIndex * 2 * newBlock->scale.y);

		newBlock->position = newBlockPosition;
		
		//Adds the block to the vector
		blocks.push_back(newBlock);

		//Moves to the next coloumn and/or row for the placement of the next block
		xIndex++;

		if (xIndex > 6)
		{
			yIndex++;
			xIndex = 0;
		}
	}

	// ----- Init textures here ----- //
	textureHandles.reserve(18); //Optimizes load process a bit more
	textureHandles.push_back(ilutGLLoadImage("img//space.jpg")); //Background texture
	textureHandles.push_back(ilutGLLoadImage("img//brick_Red.png")); //Red brick texture
	textureHandles.push_back(ilutGLLoadImage("img//brick_Blue.png")); //Blue brick texture
	textureHandles.push_back(ilutGLLoadImage("img//paddle.png")); //Paddle texture
	textureHandles.push_back(ilutGLLoadImage("img//ball.png")); //Ball texture
	textureHandles.push_back(ilutGLLoadImage("img//wall.png")); //Wall texture
	textureHandles.push_back(ilutGLLoadImage("img//gameOver.png")); //End game over text
	textureHandles.push_back(ilutGLLoadImage("img//victory.png")); //End victory text
	textureHandles.push_back(ilutGLLoadImage("img//number_0.png")); //Number 0 texture
	textureHandles.push_back(ilutGLLoadImage("img//number_1.png")); //Number 1 texture
	textureHandles.push_back(ilutGLLoadImage("img//number_2.png")); //Number 2 texture
	textureHandles.push_back(ilutGLLoadImage("img//number_3.png")); //Number 3 texture
	textureHandles.push_back(ilutGLLoadImage("img//number_4.png")); //Number 4 texture
	textureHandles.push_back(ilutGLLoadImage("img//number_5.png")); //Number 5 texture
	textureHandles.push_back(ilutGLLoadImage("img//number_6.png")); //Number 6 texture
	textureHandles.push_back(ilutGLLoadImage("img//number_7.png")); //Number 7 texture
	textureHandles.push_back(ilutGLLoadImage("img//number_8.png")); //Number 8 texture
	textureHandles.push_back(ilutGLLoadImage("img//number_9.png")); //Number 9 texture
	glBindTexture(GL_TEXTURE_2D, 0); //Binds null texture to start with

	// ----- Init effects here ----- //
	ballTrail.reserve(trailLength); //Reserves memory for the trail...helps otimize memory since we are only going to use a set number of positions which won't change

	for (int i = 0; i < trailLength; i++)
		ballTrail.push_back(glm::vec3(0.0f)); //Initializes the empty trail to start with
}

/* function DisplayCallbackFunction(void)
* Description:
*  - this is the openGL display routine
*  - this draws the sprites appropriately
*/
void DisplayCallbackFunction(void)
{
	////////////////////////////////////////////////////////////////// Clear our screen
	glClearColor(0.2f, 0.2f, 0.2f, 0.8f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	////////////////////////////////////////////////////////////////// Draw a Background
	glViewport(0, 0, windowWidth, windowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBindTexture(GL_TEXTURE_2D, textureHandles[0]);
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f); //ensure no color distortion
	glBegin(GL_QUADS);
		glTexCoord2f(1.f, 1.f);  glVertex3f(1.0, 1.0, -100.0); // ++
		glTexCoord2f(0.f, 1.f);  glVertex3f(-1.0, 1.0, -100.0); // -+
		glTexCoord2f(0.f, 0.f);  glVertex3f(-1.0, -1.0, -100.0); //--
		glTexCoord2f(1.f, 0.f);  glVertex3f(1.0, -1.0, -100.0); // +-
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	////////////////////////////////////////////////////////////////// Draw Our Scene
	glViewport(0, 0, windowWidth, windowHeight);	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-15.0f, 15.0f, -15.0f, 15.0f, 0.1f, 10000.0f);
	//gluPerspective(45.0f, 1.0f, 0.1f, 1000000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// apply camera transforms
	gluLookAt(
		cameraPosition.x,cameraPosition.y,cameraPosition.z, // camera position
		cameraPosition.x + forwardVector.x,
			cameraPosition.y + forwardVector.y,
			cameraPosition.z + forwardVector.z, // the point we're looking at
		0.0f,1.0f,0.0f  // what the camera thinks is "up"
		);

	// Draw our scene
	drawObjects();

	glutSwapBuffers();
}

/* function void KeyboardCallbackFunction(unsigned char, int,int)
* Description:
*   - this handles keyboard input when a button is pressed
*/
void KeyboardCallbackFunction(unsigned char key, int x, int y)
{
	KEYBOARD_INPUT->SetActive(key, true);

	if (key == 27)
		exit(0);
}

/* function void KeyboardUpCallbackFunction(unsigned char, int,int)
* Description:
*   - this handles keyboard input when a button is lifted
*/
void KeyboardUpCallbackFunction(unsigned char key, int x, int y)
{
	KEYBOARD_INPUT->SetActive(key, false);
}

/* function TimerCallbackFunction(int value)
* Description:
*  - this is called many times per second
*  - this enables you to animate things
*  - no drawing, just changing the state
*  - changes the frame number and calls for a redisplay
*  - FRAME_DELAY is the number of milliseconds to wait before calling the timer again
*/
void TimerCallbackFunction(int value)
{
	// ----- Control the paddle's movement using the mouse ----- //
	paddle.position.x = HelperFunctions::convertMousePosToWorld(glm::vec2(mousepositionX, mousepositionY), glm::vec2(windowWidth, windowHeight), glm::vec2(15, 15)).x;

	// ----- Accounting for fringe case where ball will either bounce perfectly horizontally or vertically, thus halting play ----- //
	if (ball.velocity.y < 5.0f && ball.velocity.y > -5.0f)
	{
		if (ball.velocity.y < 0)
			ball.velocity.y = -5.0f;
		else
			ball.velocity.y = 5.0f;
	}
	else if (ball.velocity.x < 5.0f && ball.velocity.x >-5.0f)
	{
		if (ball.velocity.x < 0)
			ball.velocity.x = -5.0f;
		else
			ball.velocity.x = 5.0f;
	}
	
	// ----- Caps the ball's speed so it doesn't become unplayable ----- //
	float ballSpeed = sqrt(ball.velocity.x * ball.velocity.x + ball.velocity.y * ball.velocity.y + ball.velocity.z * ball.velocity.z);

	if (ballSpeed > 25.0f)
		ball.velocity = glm::normalize(ball.velocity) * 25.0f;

	// ----- Speed up the ball by pressing space for instant challenge ----- //
	if (KEYBOARD_INPUT->CheckPressEvent(32))
		ball.velocity *= 2.0f;

	//----- Reset the game by pressing 'r' ----- //
	if (KEYBOARD_INPUT->CheckPressEvent('r') || KEYBOARD_INPUT->CheckPressEvent('R'))
	{
		ballTrail.clear();
		blocks.clear();
		particles.clear();
		numBallsRemaining = 3;
		score = 0;
		lastBlockHit = -1;
		timeSinceLastBlockHit = 0.0f;
		gameStage = 0;
		init();
	}

	//----- Skip to the victory screen by pressing 'v' -----//
	if (KEYBOARD_INPUT->CheckPressEvent('v') || KEYBOARD_INPUT->CheckPressEvent('V'))
	{
		for (int i = 0; i < blocks.size(); i++)
			blocks[i]->active = false;

		gameStage = 1;
	}
	
	//Clean the event list so it doesn't stay there and be considered true every frame
	KEYBOARD_INPUT->WipeEventList();

	//// update physics
	static unsigned int oldTimeSinceStart = 0;
	unsigned int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	unsigned int deltaT = timeSinceStart - oldTimeSinceStart;
	oldTimeSinceStart = timeSinceStart;

	//Convert delta t to seconds because GLUT_ELAPSED_TIME is in milliseconds (int)
	float deltaT_Seconds = float(deltaT) / 1000.0f;

	timeSinceLastBlockHit += deltaT_Seconds;

	// ---- Update Objects ---- //
	paddle.update(deltaT_Seconds);
	ball.rotation.z = atan2(ball.velocity.y, ball.velocity.x) * radToDeg; //Rotates the ball towards the direction it is moving
	ball.update(deltaT_Seconds);

	for (unsigned int i = 0; i < walls.size(); i++)
		walls[i].update(deltaT_Seconds);

	for (int i = 0; i < NUM_BLOCKS; i++)
		blocks[i]->update(deltaT_Seconds);

	updateBlockParticles(deltaT_Seconds);

	// ---- Collision Detection ---- //
	//Check if the ball went by the player and then decrement score
	if (ball.position.y < -13.5f)
	{
		numBallsRemaining--;
		std::cout << numBallsRemaining << std::endl;

		//Reset the ball if there are lives left. Otherwise end the game
		if (numBallsRemaining > 0)
		{
			glm::vec3 ballStartVelocity = HelperFunctions::randomVec3(-8.5f, 8.5f, 5.5f);
			ballStartVelocity.z = 0.0f;
			ball = GameObject(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.5f), ballStartVelocity, glm::vec3(0.0f));
			ball.colour = glm::vec3(0.5, 0.5, 0.2);
		}
		else if (gameStage != 1) //Switches to game over if the player hasn't already one
		{
			std::cout << "GAME OVER! NO LIVES REMAINING!" << std::endl;
			gameStage = 2;
			ball.position.y = -30.0f;
			ball.velocity = glm::vec3(0.0f);
		}
	}
	//Checks if the ball hits a wall and bounces it back with a higher speed
	if (ball.position.y > 13.5f) //Top wall
	{
		ball.position.y -= 0.5f;
		ball.velocity.y *= -1.05f;
	}
		
	if (ball.position.x < -13.5f) //Left wall
	{
		ball.position.x += 0.5f;
		ball.velocity.x *= -1.05f;
	}

	if (ball.position.x > 13.5f) //Right wall
	{
		ball.position.x -= 0.5f;
		ball.velocity.x *= -1.05f;
	}
		
	//Bounce ball off of player's paddle
	if (ball.position.x - ball.scale.x > paddle.position.x - paddle.scale.x && ball.position.x + ball.scale.x < paddle.position.x + paddle.scale.x)
	{
		if (ball.position.y - ball.scale.y <= paddle.position.y + paddle.scale.y)
		{
			ball.position.y += 0.5f;
			ball.velocity.y *= -1.05f;
		}
	}

	//Bounce ball off bricks
	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		if (blocks[i]->active)
		{
			if (lastBlockHit != i || timeSinceLastBlockHit >= 0.5f)
			{
				if (checkCollision(ball, *blocks[i]))
				{
					lastBlockHit = i;
					timeSinceLastBlockHit = 0.0f;

					spawnBlockParticles(blocks[i]->position, blocks[i]->singleHitType);

					collisionResponse(ball, *blocks[i]);

					if (blocks[i]->collisionDeath())
					{
						blocks[i]->active = false;
						score++;
					}
				}
			}
		}
	}

	// ---- Spawns victory fireworks if game is over ---- //
	timeSinceLastFireworks += deltaT_Seconds;

	if (gameStage == 1)
	{
		if (timeSinceLastFireworks >= 1.5f)
		{
			timeSinceLastFireworks = 0.0f;
			spawnVictoryFireworks();
		}
	}

	//Checks if all the blocks are gone and if so, switches to the victory state
	if (score == NUM_BLOCKS)
		gameStage = 1;

	//// force draw call next tick
	glutPostRedisplay();

	//// delay timestep to maintain framerate
	glutTimerFunc(FRAME_DELAY, TimerCallbackFunction, 0);
}

/* function WindowReshapeCallbackFunction()
* Description:
*  - this is called whenever the window is resized
*  - and sets up the projection matrix properly
*  - currently set up for an orthographic view (2D only)
*/
void WindowReshapeCallbackFunction(int w, int h)
{
	// switch to projection because we're changing projection
	float asp = (float)w / (float)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	windowWidth = w;
	windowHeight = h;

	//switch back to modelview
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void MouseClickCallbackFunction(int button, int state, int x, int y)
{
	// Handle mouse clicks
	if (state == GLUT_DOWN)
	{
		std::cout << "Mouse X: " << x << " Y: " << y << std::endl;
	}
}


/* function MouseMotionCallbackFunction()
* Description:
*   - this is called when the mouse is clicked and moves
*/
void MouseMotionCallbackFunction(int x, int y)
{
}

/* function MousePassiveMotionCallbackFunction()
* Description:
*   - this is called when the mouse is moved in the window
*/
void MousePassiveMotionCallbackFunction(int x, int y)
{
	mousepositionX = x;
	mousepositionY = y;
}

/* function main()
* Description:
*  - this is the main function
*  - does initialization and then calls glutMainLoop() to start the event handler
*/
int main(int argc, char **argv)
{
	// initialize the window and OpenGL properly
	glutInit(&argc, argv);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("Daniel MacCormick - 100580519 Midterm");

	// set up our function callbacks
	glutDisplayFunc(DisplayCallbackFunction); // draw
	glutKeyboardFunc(KeyboardCallbackFunction); // keyDown
	glutKeyboardUpFunc(KeyboardUpCallbackFunction); // keyUp
	glutReshapeFunc(WindowReshapeCallbackFunction); // windowResized
	glutMouseFunc(MouseClickCallbackFunction); // mouseClick
	glutMotionFunc(MouseMotionCallbackFunction); // mouseMovedActive
	glutPassiveMotionFunc(MousePassiveMotionCallbackFunction); // mouseMovedPassive
	glutTimerFunc(1, TimerCallbackFunction, 0); // timer or tick
	
	//Inits OpenGL states, gameobjects, and textures
	init();

	/* start the event handler */
	glutMainLoop();
	return 0;
}