/**
 * @file mesh.h
 * @brief 3D Mesh implementation.
 */

#pragma once

#include "node.h"
#include "face.h"
#include "octreeNode.h"
#include <string>

/**
 * @class Mesh
 * @brief Represents a drawable 3D object with geometry and material.
 */
class Mesh : public Node {
public:
    Mesh(std::vector<Face*> faces, std::vector<Vertex*> vertices);
    virtual ~Mesh();

    /**
     * @brief Renders the mesh with its material.
     * @param cameraInverse The inverse of the camera's world transformation matrix.
	 */
    void render(glm::mat4 cameraInverse) override;

private:
    std::vector<Vertex*> _vertices;
    std::vector<Face*> _faces;

    unsigned int vertexVBO = 0;
    unsigned int indexVBO = 0;
    unsigned int normalVBO = 0;

    glm::vec3 lowerBoundsCorner;
    glm::vec3 upperBoundsCorner;

    OctreeNode* rootNode;

    void renderBoundingBox();
};
