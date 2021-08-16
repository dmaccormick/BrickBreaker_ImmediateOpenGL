// loadObject.cpp
// - Wavefront OBJ loader
// - Only takes in faces with 3 points (triangles)
// - Only the most basic functionality

#include "loadObject.h"

#include <iostream>
#include <fstream>

#include <GLUT\glut.h>

void LoadObject::draw()
{
	glBegin(GL_TRIANGLES);
		glm::vec3 vert, norm;
		glm::vec2 tex;

		for (int i = 0; i < faces.size(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				vert = vertices[faces[i].vertices[j]];
				norm = normals[faces[i].normals[j]];
				tex = texcoords[faces[i].textures[j]];

				glTexCoord2f(tex.s, tex.t);
				glNormal3f(norm.x, norm.y, norm.z);
				glVertex3f(vert.x, vert.y, vert.z);
			}
		}
	glEnd();
}

bool LoadObject::loadFromObject(char* filename)
{
	std::ifstream input(filename);
	if (!input)
	{
		// error handling
		std::cerr << "Error: could not load " << filename << std::endl;
		return false;
	}

	char buf[256];
	while (input.getline(buf, 256)) //read every line
	{
		switch (buf[0])
		{
		case 'v': // v, vn, vt
			switch (buf[1])
			{
			case ' ': // v or vertex
				float x, y, z;
				sscanf_s(buf, "v %f %f %f", &x, &y, &z);
				vertices.push_back(glm::vec3(x, y, z));
				break;
			case 'n': // vn or normal
				float nx, ny, nz;
				sscanf_s(buf, "vn %f %f %f", &nx, &ny, &nz);
				normals.push_back(glm::vec3(nx, ny, nz));
				break;
			case 't': // vt or texture coordinate
				float u, v;
				sscanf_s(buf, "vt %f %f", &u, &v);
				texcoords.push_back(glm::vec2(u, v));
				break;
			}
			break;
		case 'f': // f or face
			unsigned int vertexIndices[3], uvIndices[3], normalIndices[3];
			unsigned int numMatches;

			// %u for unsigned ints, search for "scanf format string" if unsure about this
			numMatches = sscanf_s(buf, "f %u/%u/%u %u/%u/%u %u/%u/%u",
				&vertexIndices[0], &uvIndices[0], &normalIndices[0],
				&vertexIndices[1], &uvIndices[1], &normalIndices[1],
				&vertexIndices[2], &uvIndices[2], &normalIndices[2]
				);
			if (numMatches != 9) // OBJ could theoretically have less than 9 results, but no
			{
				std::cerr << "Error in file: " 
					<< filename << " numMatches wasn't 9" << std::endl;
				return false;
			}

			Face temp;

			temp.vertices[0] = vertexIndices[0] - 1;
			temp.vertices[1] = vertexIndices[1] - 1;
			temp.vertices[2] = vertexIndices[2] - 1;

			temp.textures[0] = uvIndices[0] - 1;
			temp.textures[1] = uvIndices[1] - 1;
			temp.textures[2] = uvIndices[2] - 1;

			temp.normals[0] = normalIndices[0] - 1;
			temp.normals[1] = normalIndices[1] - 1;
			temp.normals[2] = normalIndices[2] - 1;

			faces.push_back(temp);
			break;
		}
	}
	input.close();
	return true;
}