#pragma once

#include <vector>
#include <cstdint>

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
	uint32_t _id;
	std::vector<unsigned int> _indices;
	std::vector<Vertex*> _vertices;
};