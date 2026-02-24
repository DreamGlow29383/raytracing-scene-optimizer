/**
 * @file mesh.h
 * @brief 3D Mesh implementation.
 */

#pragma once

#include "node.h"
#include "material.h"

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
     * @brief Assigns a material to this mesh.
     * @param material Pointer to the material to use.
     */
    void setMaterial(Material* material);

    /**
     * @brief Checks if this mesh casts shadows.
     * @return True if it casts shadows.
     */
    bool castsShadow();

    /**
     * @brief Enables or disables shadow casting for this mesh.
     * @param castsShadow Boolean flag.
     */
    void castsShadow(bool castsShadow);

    /**
     * @brief Gets the material assigned to this mesh.
     * @return Pointer to the material.
	 */
    Material* getMaterial() const;

    /**
     * @brief Special render pass for generating shadow maps.
     * @param cameraInverse The view matrix.
     */
    void renderShadow(glm::mat4 cameraInverse);

    /**
     * @brief Renders the mesh with its material.
     * @param cameraInverse The inverse of the camera's world transformation matrix.
	 */
    void render(glm::mat4 cameraInverse) override;

private:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    bool _castsShadow;

    Material* m_material;
};
