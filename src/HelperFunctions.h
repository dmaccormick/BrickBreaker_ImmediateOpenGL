// Daniel MacCormick - 100580519
// Helper functions
// - Has basic LERP function
// - Has a function that returns a random float between two values
// - Has a function that returns a vec3 with each element being a random float between two values (essentially calls the random float function 3 times)
// - Has a function to convert mouse position in screen pixels to world space
#pragma once

#include <stdlib.h>
#include <iostream>
#include "GLM\glm.hpp"
#include <ctime>

namespace HelperFunctions
{
	template <typename T>
	static T LERP(T& P1, T& P2, float t)
	{
		return ((1 - t) * P1) + (t * P2);
	}
	
	static float LERP(float P1, float P2, float t) { //For some reason, template won't generate for floats?
		return ((1 - t) * P1) + (t * P2);
	}

	static float randomFloat(float min, float max)
	{
		if (min > max)
		{
			std::cout << "Error: Min is greater than max! Aborting!" << std::endl;
			abort();
		}
		else
		{
			float randomValue = float(rand()) / float(RAND_MAX);
			float range = max - min;

			return (randomValue * range) + min;
		}
	}

	//Like randomFloat but used almost exclusively for -min and +max. Gives a value between them that is some distance from 0 (ie: -5, 5, 2 gives a value between -5 and 5 that can't be between -2 and 2)
	static float randomFloat(float min, float max, float feather)
	{
		if(min > max)
		{
			std::cout << "Error: Min is greater than max! Aborting!" << std::endl;
			abort();
		}
		else if (feather < min || feather > max)
		{
			std::cout << "Error: Feather is not between min and max! Aborting!" << std::endl;
			abort();
		}
		else
		{
			float randomValue = float(rand()) / float(RAND_MAX);
			float range = max - min;

			float ret = (randomValue * range) + min;

			if (abs(ret) < feather)
			{
				if (ret <= 0.0f)
					ret = -feather;
				else
					ret = feather;

				return ret;
			}
			else
				return ret;
		}
	}

	static glm::vec3 randomVec3(float min, float max) {
		return glm::vec3(randomFloat(min, max), randomFloat(min, max), randomFloat(min, max));
	}

	static glm::vec3 randomVec3(float min, float max, float feather) {
		return glm::vec3(randomFloat(min, max, feather), randomFloat(min, max, feather), randomFloat(min, max, feather));
	}

	//Converts the mouse position from screen location to position location
	static glm::vec2 convertMousePosToWorld(glm::vec2 mousePosition, glm::vec2 windowDimensions, glm::vec2 orthoBounds)
	{
		glm::vec2 screenPosition;

		int halfWidth = windowDimensions.x / 2;
		int halfHeight = windowDimensions.y / 2;

		if (mousePosition.x < halfWidth)
			screenPosition.x = (-orthoBounds.x) + (mousePosition.x / halfWidth)*(orthoBounds.x);
		else if (mousePosition.x == halfWidth)
			screenPosition.x = 0.0f;
		else if (mousePosition.x > halfWidth)
			screenPosition.x = ((mousePosition.x - halfWidth) / (halfWidth)) * (orthoBounds.x);

		if (mousePosition.y < halfHeight)
			screenPosition.y = (orthoBounds.y) + (mousePosition.y / halfWidth)*(-orthoBounds.y);
		else if (mousePosition.y == halfHeight)
			screenPosition.y = 0.0f;
		else if (mousePosition.y > halfHeight)
			screenPosition.y = ((mousePosition.y - halfHeight) / (halfHeight)) * (-orthoBounds.y);

		return screenPosition;
	}
}