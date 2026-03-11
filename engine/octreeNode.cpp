#include "octreeNode.h"

OctreeNode::OctreeNode(std::vector<unsigned int>* bounding_box_corners, std::vector<Vertex*> allVertices) {

    this->allVertices = allVertices;

       glm::vec3 lowerCorner = glm::vec3(
        allVertices[(*bounding_box_corners)[0]]->x,
        allVertices[(*bounding_box_corners)[0]]->y,
        allVertices[(*bounding_box_corners)[0]]->z
    );

    glm::vec3 upperCorner = glm::vec3(
        allVertices[(*bounding_box_corners)[0]]->x,
        allVertices[(*bounding_box_corners)[0]]->y,
        allVertices[(*bounding_box_corners)[0]]->z
    );

    for (unsigned int i : *bounding_box_corners) {
        Vertex* v = allVertices[i];
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

    glm::vec3 corners[8] = {
        glm::vec3(lowerBoundsCorner.x, lowerBoundsCorner.y, lowerBoundsCorner.z), // 0
        glm::vec3(upperBoundsCorner.x, lowerBoundsCorner.y, lowerBoundsCorner.z), // 1
        glm::vec3(upperBoundsCorner.x, upperBoundsCorner.y, lowerBoundsCorner.z), // 2
        glm::vec3(lowerBoundsCorner.x, upperBoundsCorner.y, lowerBoundsCorner.z), // 3
        glm::vec3(lowerBoundsCorner.x, lowerBoundsCorner.y, upperBoundsCorner.z), // 4
        glm::vec3(upperBoundsCorner.x, lowerBoundsCorner.y, upperBoundsCorner.z), // 5
        glm::vec3(upperBoundsCorner.x, upperBoundsCorner.y, upperBoundsCorner.z), // 6
        glm::vec3(lowerBoundsCorner.x, upperBoundsCorner.y, upperBoundsCorner.z)  // 7
    };

    
    for (int i = 0; i < 8; i++) {
        m_bounding_box_corners.push_back(corners[i]);
    }
}

OctreeNode::~OctreeNode() {}

void OctreeNode::split() {

    // 1. creo 8 box
    // 2. sposto i miei vertici in esse

    float midX = (lowerBoundsCorner.x + upperBoundsCorner.x) / 2.0f;
    float midY = (lowerBoundsCorner.y + upperBoundsCorner.y) / 2.0f;
    float midZ = (lowerBoundsCorner.z + upperBoundsCorner.z) / 2.0f;
    
    std::vector<glm::vec3> childCorners[8];
    
    // Child 0: (-x, -y, -z) - bottom-left-front
    childCorners[0] = {
        glm::vec3(lowerBoundsCorner.x, lowerBoundsCorner.y, lowerBoundsCorner.z),
        glm::vec3(midX, lowerBoundsCorner.y, lowerBoundsCorner.z),
        glm::vec3(midX, midY, lowerBoundsCorner.z),
        glm::vec3(lowerBoundsCorner.x, midY, lowerBoundsCorner.z),
        glm::vec3(lowerBoundsCorner.x, lowerBoundsCorner.y, midZ),
        glm::vec3(midX, lowerBoundsCorner.y, midZ),
        glm::vec3(midX, midY, midZ),
        glm::vec3(lowerBoundsCorner.x, midY, midZ)
    };
    // Child 1: (+x, -y, -z) - bottom-right-front
    childCorners[1] = {
        glm::vec3(midX, lowerBoundsCorner.y, lowerBoundsCorner.z),
        glm::vec3(upperBoundsCorner.x, lowerBoundsCorner.y, lowerBoundsCorner.z),
        glm::vec3(upperBoundsCorner.x, midY, lowerBoundsCorner.z),
        glm::vec3(midX, midY, lowerBoundsCorner.z),
        glm::vec3(midX, lowerBoundsCorner.y, midZ),
        glm::vec3(upperBoundsCorner.x, lowerBoundsCorner.y, midZ),
        glm::vec3(upperBoundsCorner.x, midY, midZ),
        glm::vec3(midX, midY, midZ)
    };
    
    // Child 2: (+x, +y, -z) - bottom-right-back
    childCorners[2] = {
        glm::vec3(midX, midY, lowerBoundsCorner.z),
        glm::vec3(upperBoundsCorner.x, midY, lowerBoundsCorner.z),
        glm::vec3(upperBoundsCorner.x, upperBoundsCorner.y, lowerBoundsCorner.z),
        glm::vec3(midX, upperBoundsCorner.y, lowerBoundsCorner.z),
        glm::vec3(midX, midY, midZ),
        glm::vec3(upperBoundsCorner.x, midY, midZ),
        glm::vec3(upperBoundsCorner.x, upperBoundsCorner.y, midZ),
        glm::vec3(midX, upperBoundsCorner.y, midZ)
    };
    
    // Child 3: (-x, +y, -z) - bottom-left-back
    childCorners[3] = {
        glm::vec3(lowerBoundsCorner.x, midY, lowerBoundsCorner.z),
        glm::vec3(midX, midY, lowerBoundsCorner.z),
        glm::vec3(midX, upperBoundsCorner.y, lowerBoundsCorner.z),
        glm::vec3(lowerBoundsCorner.x, upperBoundsCorner.y, lowerBoundsCorner.z),
        glm::vec3(lowerBoundsCorner.x, midY, midZ),
        glm::vec3(midX, midY, midZ),
        glm::vec3(midX, upperBoundsCorner.y, midZ),
        glm::vec3(lowerBoundsCorner.x, upperBoundsCorner.y, midZ)
    };
    
    // Child 4: (-x, -y, +z) - top-left-front
    childCorners[4] = {
        glm::vec3(lowerBoundsCorner.x, lowerBoundsCorner.y, midZ),
        glm::vec3(midX, lowerBoundsCorner.y, midZ),
        glm::vec3(midX, midY, midZ),
        glm::vec3(lowerBoundsCorner.x, midY, midZ),
        glm::vec3(lowerBoundsCorner.x, lowerBoundsCorner.y, upperBoundsCorner.z),
        glm::vec3(midX, lowerBoundsCorner.y, upperBoundsCorner.z),
        glm::vec3(midX, midY, upperBoundsCorner.z),
        glm::vec3(lowerBoundsCorner.x, midY, upperBoundsCorner.z)
    };
    
    // Child 5: (+x, -y, +z) - top-right-front
    childCorners[5] = {
        glm::vec3(midX, lowerBoundsCorner.y, midZ),
        glm::vec3(upperBoundsCorner.x, lowerBoundsCorner.y, midZ),
        glm::vec3(upperBoundsCorner.x, midY, midZ),
        glm::vec3(midX, midY, midZ),
        glm::vec3(midX, lowerBoundsCorner.y, upperBoundsCorner.z),
        glm::vec3(upperBoundsCorner.x, lowerBoundsCorner.y, upperBoundsCorner.z),
        glm::vec3(upperBoundsCorner.x, midY, upperBoundsCorner.z),
        glm::vec3(midX, midY, upperBoundsCorner.z)
    };
    
    // Child 6: (+x, +y, +z) - top-right-back
    childCorners[6] = {
        glm::vec3(midX, midY, midZ),
        glm::vec3(upperBoundsCorner.x, midY, midZ),
        glm::vec3(upperBoundsCorner.x, upperBoundsCorner.y, midZ),
        glm::vec3(midX, upperBoundsCorner.y, midZ),
        glm::vec3(midX, midY, upperBoundsCorner.z),
        glm::vec3(upperBoundsCorner.x, midY, upperBoundsCorner.z),
        glm::vec3(upperBoundsCorner.x, upperBoundsCorner.y, upperBoundsCorner.z),
        glm::vec3(midX, upperBoundsCorner.y, upperBoundsCorner.z)
    };
    
    // Child 7: (-x, +y, +z) - top-left-back
    childCorners[7] = {
        glm::vec3(lowerBoundsCorner.x, midY, midZ),
        glm::vec3(midX, midY, midZ),
        glm::vec3(midX, upperBoundsCorner.y, midZ),
        glm::vec3(lowerBoundsCorner.x, upperBoundsCorner.y, midZ),
        glm::vec3(lowerBoundsCorner.x, midY, upperBoundsCorner.z),
        glm::vec3(midX, midY, upperBoundsCorner.z),
        glm::vec3(midX, upperBoundsCorner.y, upperBoundsCorner.z),
        glm::vec3(lowerBoundsCorner.x, upperBoundsCorner.y, upperBoundsCorner.z)
    };
    
    for (int i = 0; i < 8; i++) {
        children[i] = new OctreeNode(&childCorners[i], allVertices);
    }
    
    for (Face* face : faces) {
        for (int i = 0; i < 8; i++) {
            if (children[i]->check(face)) {
                children[i]->insert(face);
            }
        }
    }
    
    faces.clear();
    m_isSplit = true;
}

void OctreeNode::insert(Face* face) {
    if (this->faces.size() >= max_faces) {
        split();

        for (int i = 0; i < 8; i++)
        {
            if (this->children[i]->check(face)) {
                this->children[i]->insert(face);
            }
        }
        
    }
    this->faces.push_back(face);
}

bool OctreeNode::check(const Face* face) {
    for (const int& i: face->_indices)
    {
        Vertex *v = allVertices.at(i); // 1 of 3 inside the box is enough
        if (v->x >= lowerBoundsCorner.x && v->y >= lowerBoundsCorner.y && v->z >= lowerBoundsCorner.z
        && v->x <= upperBoundsCorner.x && v->y <= upperBoundsCorner.y && v->z <= upperBoundsCorner.z) {
            return true;
        }
    }
    return false;
}

bool OctreeNode::isSplit() {
    return this->m_isSplit;
}
