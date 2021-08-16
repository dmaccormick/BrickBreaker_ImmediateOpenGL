// loadObject.h
// provided by TA
// - Wavefront OBJ loader
// - Only takes in faces with 3 points (triangles)
// - Only the most basic functionality

#pragma once

#include <GLM\glm.hpp>
#include <vector>

struct Face
{
	Face() {}

	// these are arrays of size 3 to represent 3 points of a triangle
	// these values represent the indices of the points
	unsigned int normals[3], 
		vertices[3], 
		textures[3];
};

// we're loading a Wavefront OBJ
class LoadObject
{
public:
	bool loadFromObject(char* filename);
	void draw();
private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords; //note: vec2
	std::vector<Face> faces;
};