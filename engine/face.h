#pragma once

#include <vector>

/**
 * @struct Vertex
 * @brief Represents a single vertex with position, normal, and texture coordinates.
 */
struct Vertex
{
	float x, y, z;
	float nx, ny, nz;
	float u, v;
};

struct Face
{
	std::vector<unsigned int> _indices;
	std::vector<Vertex*> _vertices;
};