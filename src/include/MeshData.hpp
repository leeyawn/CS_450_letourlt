#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <iostream>
#include <vector>
#include "glm/glm.hpp"
using namespace std;

// Struct for holding vertex data
struct Vertex {
	glm::vec3 position;
	glm::vec4 color;
	glm::vec3 normal = glm::vec3(0,0,0);
	glm::vec2 texcoord = glm::vec2(0,0);
	glm::vec3 tangent = glm::vec3(1,0,0);
};

// Struct for holding mesh data
struct Mesh {
	vector<Vertex> vertices;
	vector<unsigned int> indices;
};

#endif
