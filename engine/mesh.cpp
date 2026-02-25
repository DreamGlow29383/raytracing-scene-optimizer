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