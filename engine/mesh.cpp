#include "mesh.h"

#include <iostream>

Mesh::Mesh()
{
    Mesh::Node();
	m_material = nullptr;
}

Mesh::~Mesh()
{

}


Material* Mesh::getMaterial() const
{
    return m_material;
}

void Mesh::setMaterial(Material* material)
{
    m_material = material;
}

void Mesh::setVertices(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
    m_vertices = vertices;
    m_indices = indices;
}

void Mesh::renderShadow(glm::mat4 cameraInverse)
{
    if (!_castsShadow)
        return;

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(cameraInverse));

    glBegin(GL_TRIANGLES);
    for (unsigned int i : m_indices) {
        const Vertex& v = m_vertices[i];
        glVertex3f(v.x, v.y, v.z);
    }
    glEnd();
}

bool Mesh::castsShadow() {
    return _castsShadow;
}

void Mesh::castsShadow(bool castsShadow) {
    _castsShadow = castsShadow;
}

void Mesh::render(glm::mat4 cameraInverse)
{
    if (m_material) m_material->render(cameraInverse);

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

    if (m_material) glDisable(GL_TEXTURE_2D);
}