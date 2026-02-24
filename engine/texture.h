/**
 * @file texture.h
 * @brief Texture resource management.
 */

#pragma once

#define FREEIMAGE_LIB
#define GL_TEXTURE_MAX_ANISOTROPY_EXT        0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT    0x84FF

#include "object.h"

#include <GL/freeglut.h> 
#include <FreeImage.h>
#include <string>

 /**
  * @class Texture
  * @brief Encapsulates an OpenGL texture object.
  * * Handles loading from file (via FreeImage) and binding to the GPU.
  */
class Texture : public Object
{
public:
    Texture();
    ~Texture();

    /**
     * @brief Loads raw texture data into OpenGL.
     * @param data Pointer to the raw byte array.
     * @param size Size of the data array.
     */
    void setTextureData(const unsigned char* data, size_t size);

    /**
     * @brief Gets the OpenGL ID of the texture.
     * @return GLuint handle.
     */
    GLuint getID() const;

    /**
	 * @brief Gets the width of the texture in pixels.
	 * @return Width in pixels.
     */
    int getWidth() const;

    /**
	 * @brief Gets the height of the texture in pixels.
	 * @return Height in pixels.
     */
    int getHeight() const;

    /**
     * @brief Binds the texture for rendering.
     * @param cameraInverse Unused in this context, but required by Object interface.
     */
    virtual void render(glm::mat4 cameraInverse) override;

private:
    bool createTextureFromBitmap(FIBITMAP* bitmap);

    GLuint m_texId;
    int m_width;
    int m_height;
};