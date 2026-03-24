#include "octreeNode.h"

#include <utility>
#include <iostream>

OctreeNode::OctreeNode(std::vector<Vertex*> allVertices, std::vector<Face*> allFaces) {

    glm::vec3 lowerCorner = glm::vec3(allVertices[0]->x, allVertices[0]->y, allVertices[0]->z);
    glm::vec3 upperCorner = glm::vec3(allVertices[0]->x, allVertices[0]->y, allVertices[0]->z);

    for (Vertex* v : allVertices) {
        if (v->x < lowerCorner.x)
            lowerCorner.x = v->x;
        if (v->y < lowerCorner.y)
            lowerCorner.y = v->y;
        if (v->z < lowerCorner.z)
            lowerCorner.z = v->z;

        if (v->x > upperCorner.x)
            upperCorner.x = v->x;
        if (v->y > upperCorner.y)
            upperCorner.y = v->y;
        if (v->z > upperCorner.z)
            upperCorner.z = v->z;
    }

    lowerBoundsCorner = lowerCorner;
    upperBoundsCorner = upperCorner;

    float width = abs(upperBoundsCorner.x - lowerBoundsCorner.x);
    float height = abs(upperBoundsCorner.y - lowerBoundsCorner.y);
    float depth = abs(upperBoundsCorner.z - lowerBoundsCorner.z);

    float longestSide = width;
    if (height > longestSide) {
        longestSide = height;
    }
    if (depth > longestSide) {
        longestSide = depth;
    }

    float newMaxX = lowerBoundsCorner.x + longestSide;
    float newMaxY = lowerBoundsCorner.y + longestSide;
    float newMaxZ = lowerBoundsCorner.z + longestSide;

    upperBoundsCorner = glm::vec3(newMaxX, newMaxY, newMaxZ);

    for (Face* face : allFaces)
        insert(face);
}

OctreeNode::OctreeNode(glm::vec3 lowerCorner, glm::vec3 upperCorner, int depth) {
    lowerBoundsCorner = lowerCorner;
    upperBoundsCorner = upperCorner;
    node_depth = depth;
}

OctreeNode::~OctreeNode() {}

void OctreeNode::split() {

    // 1. creo 8 box
    // 2. sposto i miei vertici in esse

    float midX = (lowerBoundsCorner.x + upperBoundsCorner.x) / 2.0f;
    float midY = (lowerBoundsCorner.y + upperBoundsCorner.y) / 2.0f;
    float midZ = (lowerBoundsCorner.z + upperBoundsCorner.z) / 2.0f;
    
    float sideLength = abs(lowerBoundsCorner.x - upperBoundsCorner.x) / 2;

    std::vector<std::pair<glm::vec3, glm::vec3>> childCorners;

    for (int zdiff = 0; zdiff < 2; zdiff++) {
        for (int ydiff = 0; ydiff < 2; ydiff++) {
            for (int xdiff = 0; xdiff < 2; xdiff++) {
                glm::vec3 corner1 = glm::vec3(
                    lowerBoundsCorner.x + (sideLength * xdiff),
                    lowerBoundsCorner.y + (sideLength * ydiff),
                    lowerBoundsCorner.z + (sideLength * zdiff)
                );
                glm::vec3 corner2 = glm::vec3(corner1.x + sideLength, corner1.y + sideLength, corner1.z + sideLength);

                childCorners.push_back({ corner1, corner2 });
            }
        }
    }

    for (int i = 0; i < childCorners.size(); i++) {
        children.push_back(new OctreeNode(childCorners[i].first, childCorners[i].second, node_depth + 1));
    }
    
    int n = 0;
    for (Face* face : faces) {
        for (int i = 0; i < children.size(); i++) {
            if (children[i]->check(face)) {
                children[i]->insert(face);
                n++;
            }
        }
    }

    if (n < faces.size())
        std::cout << "Warning: not all faces were inherited by children" << std::endl;
    
    faces.clear();
    m_isSplit = true;
}

void OctreeNode::insert(Face* face) {
    if (m_isSplit) {
        for (OctreeNode* child : children)
            if (child->check(face))
                child->insert(face);
    }
    else if (this->faces.size() >= max_faces && node_depth < max_depth) {
        split();
        for (OctreeNode* child : children)
            if (child->check(face))
                child->insert(face);
    }
    else {
        this->faces.push_back(face);
    }
}

bool OctreeNode::check(const Face* face) {
    for (Vertex* v: face->_vertices)
    {
        // small epsilon to avoid floating point errors
        const float EPSILON = 0.0001f;
        if (v->x >= lowerBoundsCorner.x - EPSILON && 
            v->y >= lowerBoundsCorner.y - EPSILON && 
            v->z >= lowerBoundsCorner.z - EPSILON &&
            v->x <= upperBoundsCorner.x + EPSILON && 
            v->y <= upperBoundsCorner.y + EPSILON && 
            v->z <= upperBoundsCorner.z + EPSILON) {
            return true;
        }
    }
    return false;
}

bool OctreeNode::isSplit() {
    return this->m_isSplit;
}
