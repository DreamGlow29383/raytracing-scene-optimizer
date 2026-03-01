#include "mesh.h"

#include <iostream>
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

    glBegin(GL_TRIANGLES);
    for (unsigned int i : m_indices)
    {
        const Vertex& v = m_vertices[i];

        glNormal3f(v.nx, v.ny, v.nz);
        glTexCoord2f(v.u, v.v);
        glVertex3f(v.x, v.y, v.z);
    }
    glEnd();
}