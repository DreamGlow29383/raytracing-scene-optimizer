#include "material.h"

#include <iostream>

Material::Material()
    : m_shininess(32.0f), m_texture(nullptr), m_texScale(1.0f, 1.0f)
{
    //Default colors
    m_ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    m_diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
    m_specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

Material::~Material()
{
}

void Material::setTexture(Texture* texture)
{
    m_texture = texture;
}

void Material::setTextureScale(float x, float y)
{
    m_texScale = glm::vec2(x, y);
}

void Material::setAmbient(const glm::vec3& color)
{
    m_ambient = glm::vec4(color, 1.0f);
}

void Material::setDiffuse(const glm::vec3& color)
{
    m_diffuse = glm::vec4(color, 1.0f);
}

void Material::setSpecular(const glm::vec3& color)
{
    m_specular = glm::vec4(color, 1.0f);
}

void Material::setShininess(float shininess)
{
    m_shininess = shininess;
}

void Material::render(glm::mat4 cameraInverse)
{

    glMatrixMode(GL_TEXTURE);
	glLoadMatrixf(glm::value_ptr(glm::mat4(1.0f)));

    glScalef(m_texScale.x, m_texScale.y, 1.0f);

    glMatrixMode(GL_MODELVIEW);

    if (m_texture)
    {
        m_texture->render(cameraInverse);
        glEnable(GL_TEXTURE_2D);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glm::value_ptr(m_ambient));
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glm::value_ptr(m_diffuse));
    }

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glm::value_ptr(m_specular));
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_shininess);
}