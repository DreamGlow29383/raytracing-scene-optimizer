#include "mesh.h"

#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>

Mesh::Mesh()
{
    Mesh::Node();
}

Mesh::~Mesh()
{

}

void Mesh::setVertices(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
    m_vertices = vertices;
    m_indices = indices;

    float* flatVertices = new float[m_vertices.size() * 3];
    float* flatNormals = new float[m_vertices.size() * 3];
    for (int i = 0; i < m_vertices.size(); i++) {
        flatVertices[i * 3] = m_vertices[i].x;
        flatVertices[i * 3 + 1] = m_vertices[i].y;
        flatVertices[i * 3 + 2] = m_vertices[i].z;

        flatNormals[i * 3] = m_vertices[i].nx;
        flatNormals[i * 3 + 1] = m_vertices[i].ny;
        flatNormals[i * 3 + 2] = m_vertices[i].nz;
    }

    glGenBuffers(1, &vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * 3 * sizeof(float), flatVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &indexVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * 3 * sizeof(float), flatNormals, GL_STATIC_DRAW);

    delete[] flatVertices;
    delete[] flatNormals;

    glm::vec3 lowerCorner = glm::vec3(m_vertices[0].x, m_vertices[0].y, m_vertices[0].z);
    glm::vec3 upperCorner = glm::vec3(m_vertices[0].x, m_vertices[0].y, m_vertices[0].z);

    for (Vertex v : m_vertices) {
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

    std::cout << "Lower Corner Coords: " << std::endl;
    std::cout << "X: " << lowerCorner.x << std::endl;
    std::cout << "Y: " << lowerCorner.y << std::endl;
    std::cout << "Z: " << lowerCorner.z << std::endl;

    std::cout << "Upper Corner Coords: " << std::endl;
    std::cout << "X: " << upperCorner.x << std::endl;
    std::cout << "Y: " << upperCorner.y << std::endl;
    std::cout << "Z: " << upperCorner.z << std::endl;

    lowerBoundsCorner = lowerCorner;
    upperBoundsCorner = upperCorner;
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
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}