#include "texture.h"
#include <iostream>

Texture::Texture() : m_texId(0), m_width(0), m_height(0)
{

}

Texture::~Texture()
{
    if (m_texId != 0)
    {
        glDeleteTextures(1, &m_texId);
    }
}

GLuint Texture::getID() const
{
    return m_texId;
}

int Texture::getWidth() const
{
    return m_width;
}

int Texture::getHeight() const
{
    return m_height;
}

void Texture::setTextureData(const unsigned char* data, size_t size)
{
    if (!data || size == 0) return;

    FIMEMORY* hmem = FreeImage_OpenMemory((unsigned char*) data, (unsigned int) size);
    if (!hmem) return;

    FIBITMAP* bitmap = nullptr;
    FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromMemory(hmem, 0);
    if (format != FIF_UNKNOWN)
    {
        bitmap = FreeImage_LoadFromMemory(format, hmem, 0);
    }

    if (bitmap)
    {
        std::cout << "[Texture] Texture decoded successfully" << std::endl;
        createTextureFromBitmap(bitmap);
        FreeImage_Unload(bitmap);
    }
    else
    {
        std::cerr << "[Texture] Error: Failed to decode texture data." << std::endl;
    }

    FreeImage_CloseMemory(hmem);
}

bool Texture::createTextureFromBitmap(FIBITMAP* bitmap)
{
    FIBITMAP* bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
    if (!bitmap32) return false;

    m_width = FreeImage_GetWidth(bitmap32);
    m_height = FreeImage_GetHeight(bitmap32);
    void* data = FreeImage_GetBits(bitmap32);

    if (m_texId) glDeleteTextures(1, &m_texId);
    glGenTextures(1, &m_texId);
    glBindTexture(GL_TEXTURE_2D, m_texId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    float maxAnisotropy = 1.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, data);

    FreeImage_Unload(bitmap32);

    return true;
}

void Texture::render(glm::mat4 cameraInverse)
{
    if (m_texId != 0)
    {
        glBindTexture(GL_TEXTURE_2D, m_texId);
    }
}