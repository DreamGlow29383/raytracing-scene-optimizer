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
    Mesh(std::vector<Face*> faces, std::vector<Vertex*> vertices, OctreeNode* octreeRoot);
    virtual ~Mesh();

    /**
     * @brief Renders the mesh with its material.
     * @param cameraInverse The inverse of the camera's world transformation matrix.
	 */
    void render(glm::mat4 cameraInverse) override;

    OctreeNode* getOctreeRoot();
    std::vector<Face*> getFaces() {
        return _faces;
    }

    void updateColorVBO(int coloringMode);

    void setFaceHit(Face* face) {
       _faceHit = face;
    }

    void setNodesHit(std::vector<OctreeNode*> nodes) {
       _nodesHit = nodes;
    }

private:
    std::vector<Vertex*> _vertices;
    std::vector<Face*> _faces;

    Face* _faceHit;
    std::vector<OctreeNode*> _nodesHit;

    unsigned int vertexVBO = 0;
    unsigned int indexVBO = 0;
    unsigned int normalVBO = 0;

    unsigned int expandedVertexVBO = 0;
    unsigned int expandedNormalVBO = 0;
    unsigned int colorVBO = 0;
    unsigned int unifiedIndexVBO = 0;
    unsigned int unifiedIndexCount = 0;

    struct NodeRange {
        unsigned int start;
        unsigned int count;
    };
    std::unordered_map<OctreeNode*, NodeRange> nodeRanges;

    std::unordered_map<OctreeNode*, unsigned int> nodeIndexVBOs;
    std::unordered_map<OctreeNode*, int> nodeIndexCounts;
    std::unordered_map<OctreeNode*, glm::vec3> depthColors;
    std::unordered_map<OctreeNode*, glm::vec3> faceColors;
    std::unordered_map<OctreeNode*, glm::vec3> nodeColors;

    OctreeNode* rootNode;
    std::vector<OctreeNode*> _leafNodes;

    void generateMesh(std::vector<Face*> faces, std::vector<Vertex*> vertices);
    void buildExpandedBuffers();

    void renderOctree(OctreeNode* node);
    void printOctreeHierarchy(OctreeNode* node, const std::string& prefix, bool isLast, bool isRoot);
    glm::vec3 computeDensityColor(size_t faceCount);
    glm::vec3 computeDepthColor(int depth);
    glm::vec3 computeRandomColor();
};
