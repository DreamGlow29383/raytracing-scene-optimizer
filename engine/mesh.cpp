#include "mesh.h"

#include <iostream>
#include <math.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

Mesh::Mesh(std::vector<Face> faces, std::vector<Vertex> vertices)
{
    Mesh::Node();

    _faces = faces;
    _vertices = vertices;

    std::vector<unsigned int> _indices;
    for (Face face : faces) {
        for (unsigned int index : face._indices)
            _indices.push_back(index);
    }

    float* flatVertices = new float[_vertices.size() * 3];
    float* flatNormals = new float[_vertices.size() * 3];
    for (int i = 0; i < _vertices.size(); i++) {
        flatVertices[i * 3] = _vertices[i].x;
        flatVertices[i * 3 + 1] = _vertices[i].y;
        flatVertices[i * 3 + 2] = _vertices[i].z;

        flatNormals[i * 3] = _vertices[i].nx;
        flatNormals[i * 3 + 1] = _vertices[i].ny;
        flatNormals[i * 3 + 2] = _vertices[i].nz;
    }

    glGenBuffers(1, &vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * 3 * sizeof(float), flatVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &indexVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), _indices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * 3 * sizeof(float), flatNormals, GL_STATIC_DRAW);

    delete[] flatVertices;
    delete[] flatNormals;

    glm::vec3 lowerCorner = glm::vec3(_vertices[0].x, _vertices[0].y, _vertices[0].z);
    glm::vec3 upperCorner = glm::vec3(_vertices[0].x, _vertices[0].y, _vertices[0].z);

    for (Vertex v : _vertices) {
        if (v.x < lowerCorner.x)
            lowerCorner.x = v.x;
        if (v.y < lowerCorner.y)
            lowerCorner.y = v.y;
        if (v.z < lowerCorner.z)
            lowerCorner.z = v.z;

        if (v.x > upperCorner.x)
            upperCorner.x = v.x;
        if (v.y > upperCorner.y)
            upperCorner.y = v.y;
        if (v.z > upperCorner.z)
            upperCorner.z = v.z;
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
}

Mesh::~Mesh()
{

}

void Mesh::render(glm::mat4 cameraInverse)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(cameraInverse * this->getWC()));

    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glVertexPointer(3, GL_FLOAT, 0, nullptr);
    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glNormalPointer(GL_FLOAT, 0, nullptr);
    glEnableClientState(GL_NORMAL_ARRAY);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
    glDrawElements(GL_TRIANGLES, _faces.size() * 3, GL_UNSIGNED_INT, nullptr);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    renderBoundingBox();
}

void Mesh::renderBoundingBox()
{
    // Save current state
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // Disable lighting for bounding box
    glDisable(GL_LIGHTING);

    // Set line properties
    glLineWidth(2.0f);
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for bounding box

    // Get bounding box corners
    glm::vec3 min = lowerBoundsCorner;
    glm::vec3 max = upperBoundsCorner;

    // Define the 8 corners of the bounding box
    glm::vec3 corners[8] = {
        glm::vec3(min.x, min.y, min.z), // 0
        glm::vec3(max.x, min.y, min.z), // 1
        glm::vec3(max.x, max.y, min.z), // 2
        glm::vec3(min.x, max.y, min.z), // 3
        glm::vec3(min.x, min.y, max.z), // 4
        glm::vec3(max.x, min.y, max.z), // 5
        glm::vec3(max.x, max.y, max.z), // 6
        glm::vec3(min.x, max.y, max.z)  // 7
    };

    // Define the 12 edges of the bounding box (pairs of corner indices)
    int edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Bottom face
        {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Top face
        {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Vertical edges
    };

    // Draw the bounding box lines
    glBegin(GL_LINES);
    for (int i = 0; i < 12; i++) {
        glVertex3f(corners[edges[i][0]].x, corners[edges[i][0]].y, corners[edges[i][0]].z);
        glVertex3f(corners[edges[i][1]].x, corners[edges[i][1]].y, corners[edges[i][1]].z);
    }
    glEnd();

    // Optionally draw corner points for better visibility
    glPointSize(4.0f);
    glColor3f(0.0f, 1.0f, 0.0f); // Green for points
    glBegin(GL_POINTS);
    for (int i = 0; i < 8; i++) {
        glVertex3f(corners[i].x, corners[i].y, corners[i].z);
    }
    glEnd();

    // Restore previous state
    glPopAttrib();
}