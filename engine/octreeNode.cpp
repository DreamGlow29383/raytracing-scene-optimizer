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

    const float EPSILON = 0.01f;

    lowerBoundsCorner.x -= EPSILON;
    lowerBoundsCorner.y -= EPSILON;
    lowerBoundsCorner.z -= EPSILON;

    upperBoundsCorner.x += EPSILON;
    upperBoundsCorner.y += EPSILON;
    upperBoundsCorner.z += EPSILON;

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

    this->id = 000;
}

OctreeNode::OctreeNode(glm::vec3 lowerCorner, glm::vec3 upperCorner, int depth, int id) {
    this->lowerBoundsCorner = lowerCorner;
    this->upperBoundsCorner = upperCorner;
    this->node_depth = depth;
    this->id = id;
}

OctreeNode::~OctreeNode() {
    for (OctreeNode* child : children)
        delete(child);
    children.clear();
}

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

    for (unsigned char i = 0; i < childCorners.size(); i++) {
        children.push_back(new OctreeNode(childCorners[i].first, childCorners[i].second, node_depth + 1, i));
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
}

void OctreeNode::insert(Face* face) {
    if (hasChildren()) {
        for (OctreeNode* child : children)
            if (child->check(face))
                child->insert(face);
    }
    else if (this->faces.size() >= MAX_FACES && node_depth < MAX_DEPTH) {
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

   // small epsilon to avoid floating point errors
   const float EPSILON = 0.0001f;

    for (Vertex* v: face->_vertices)
    {
        // check if vertices are inside the box
        if (v->x >= lowerBoundsCorner.x - EPSILON && 
            v->y >= lowerBoundsCorner.y - EPSILON && 
            v->z >= lowerBoundsCorner.z - EPSILON &&
            v->x <= upperBoundsCorner.x + EPSILON && 
            v->y <= upperBoundsCorner.y + EPSILON && 
            v->z <= upperBoundsCorner.z + EPSILON) {
            return true;
        }
    }

    // Check if any edge of the triangle intersects the cube
    for (int i = 0; i < 3; i++) {
       Vertex* p1 = face->_vertices[i];
       Vertex* p2 = face->_vertices[(i + 1) % 3];

       // Liang-Barsky algorithm for line intersection
       float t0 = 0.0f, t1 = 1.0f;
       float dx = p2->x - p1->x;
       float dy = p2->y - p1->y;
       float dz = p2->z - p1->z;

       float p[6] = { -dx, dx, -dy, dy, -dz, dz };
       float q[6] = { p1->x - lowerBoundsCorner.x, upperBoundsCorner.x - p1->x,
                     p1->y - lowerBoundsCorner.y, upperBoundsCorner.y - p1->y,
                     p1->z - lowerBoundsCorner.z, upperBoundsCorner.z - p1->z };

       bool intersect = true;
       for (int j = 0; j < 6; j++) {
          if (p[j] == 0) {
             if (q[j] < 0) { intersect = false; break; }
          }
          else {
             float t = q[j] / p[j];
             if (p[j] < 0) {
                if (t > t0) t0 = t;
             }
             else {
                if (t < t1) t1 = t;
             }
          }
       }

       if (intersect && t0 <= t1) return true;
    }

    return false;
}

void OctreeNode::addChild(OctreeNode* node) {
    children.push_back(node);
}