/**
 * @file mesh.h
 * @brief 3D Mesh implementation.
 */

#pragma once

#include "node.h"
#include <string>

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

/**
 * @class Mesh
 * @brief Represents a drawable 3D object with geometry and material.
 */
class Mesh : public Node {
public:
    Mesh();
    virtual ~Mesh();

    /**
     * @brief Sets the geometry data for the mesh.
     * @param vertices List of vertices containing position, normal, and UV.
     * @param indices List of indices defining the triangles.
     */
    void setVertices(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

    /**
     * @brief Renders the mesh with its material.
     * @param cameraInverse The inverse of the camera's world transformation matrix.
	 */
    void render(glm::mat4 cameraInverse) override;

private:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;

    unsigned int vertexVBO = 0;
    unsigned int indexVBO = 0;
    unsigned int normalVBO = 0;

    glm::vec3 lowerBoundsCorner;
    glm::vec3 upperBoundsCorner;

    void renderBoundingBox();
};
