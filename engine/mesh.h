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

	OctreeNode* getOctreeRoot();

private:

	std::vector<OctreeNode*> colorType;
	std::vector<Face*> _facesHit;
	std::vector<OctreeNode*> _nodesHit;

	glm::vec3 colors[5];

	std::vector<Vertex*> _vertices;
	std::vector<Face*> _faces;

	unsigned int vertexVBO = 0;
	unsigned int indexVBO = 0;
	unsigned int normalVBO = 0;

	std::unordered_map<OctreeNode*, unsigned int> nodeIndexVBOs;
	std::unordered_map<OctreeNode*, int> nodeIndexCounts;
	std::unordered_map<OctreeNode*, glm::vec3> nodeColors;

	OctreeNode* rootNode;

	void renderOctree(OctreeNode* node);
	void printOctreeHierarchy(OctreeNode* node, const std::string& prefix, bool isLast, bool isRoot);
	glm::vec3 computeDensityColor(size_t faceCount);
	glm::vec3 computeDepthColor(int depth);
};
