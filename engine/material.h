/**
 * @file material.h
 * @brief Material properties for shading.
 */

#pragma once

#include "texture.h"
#include "object.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

 /**
  * @class Material
  * @brief Defines the optical properties of a mesh surface.
  */
class Material : public Object {
public:
    Material();
    ~Material();

    /**
     * @brief Assigns a texture to the material.
     * @param texture Pointer to the Texture object.
     */
    void setTexture(Texture* texture);

    /**
     * @brief Sets the tiling scale of the texture.
     * @param x Scale factor in X (U).
     * @param y Scale factor in Y (V).
     */
    void setTextureScale(float x, float y);

    /**
	 * @brief Sets the ambient, diffuse, specular colors and shininess factor.
	 * @param color Ambient color.
	 * @param color Diffuse color.
	 * @param color Specular color.
     */
    void setAmbient(const glm::vec3& color);
    void setDiffuse(const glm::vec3& color);
    void setSpecular(const glm::vec3& color);

    /**
     * @brief Sets the specular shininess (exponent).
     * Higher values mean smaller, sharper highlights.
     * @param shininess The shininess factor.
     */
    void setShininess(float shininess);

    /**
     * @brief Renders the material (sets shader uniforms).
     * @param cameraInverse The inverse of the camera's world transformation matrix.
	 */
	virtual void render(glm::mat4 cameraInverse) override;

private:
    glm::vec2 m_texScale;
    glm::vec3 m_ambient;
    glm::vec3 m_diffuse;
    glm::vec3 m_specular;
    float m_shininess;

    Texture* m_texture;
};