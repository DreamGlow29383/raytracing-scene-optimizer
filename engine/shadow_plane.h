/**
 * @file shadow_plane.h
 * @brief Planar shadow implementation.
 */

#pragma once

#include "node.h"
#include "light.h"
#include <vector>

 /**
  * @class ShadowPlane
  * @brief Defines a plane onto which shadows are projected.
  * * Uses planar projection matrices to flatten geometry onto this surface.
  */
class ShadowPlane : public Node {
public:
    ShadowPlane();
    ~ShadowPlane();

    /**
     * @brief Sets the radius of the shadow plane (if circular).
     * @param radius The radius value.
     */
    void setRadius(float radius);

    /**
	 * @brief Gets the radius of the shadow plane.
	 * @return The radius value.
     */
    float getRadius() const;

    /**
     * @brief Sets the type of shadow plane (ENDLESS or CIRCULAR).
     * @param type The shadow plane type.
	 */
    void setType(Eng::ShadowPlaneType type);

    /**
	 * @brief Gets the type of shadow plane.
	 * @return The shadow plane type.
     */
    Eng::ShadowPlaneType getType() const;

    /**
     * @brief Sets the color of the shadows cast on this plane.
     * @param color RGBA color (alpha controls darkness).
     */
    void setShadowColor(const glm::vec4& color);

    /**
	 * @brief Gets the color of the shadows cast on this plane.
	 * @return RGBA color vector.
     */
    glm::vec4 getShadowColor() const;

    /**
     * @brief Sets a slight vertical offset to prevent Z-fighting.
     * @param offset The offset value (lift) from the ground.
     */
    void setShadowOffset(float offset);  // To prevent z-fighting

    /**
	 * @brief Gets the shadow offset value.
	 * @return The offset value.
     */
    float getShadowOffset() const;

    // Main rendering method
    /**
	 * @brief Renders the shadow plane.
	 * @param cameraInverse The inverse of the camera's world transformation matrix.
     */
    void render(glm::mat4 cameraInverse) override;

    // Method to compute shadow matrix for a specific light
    /**
     * @brief Computes the projection matrix to squash geometry onto this plane.
     * @param light Pointer to the light source casting the shadow.
     * @return A 4x4 shadow projection matrix.
     */
    glm::mat4 computeShadowMatrixForLight(Light* light) const;

    // Check if a point is within this shadow plane's influence
    /**
     * @brief Checks if a 3D point lies within the boundaries of this plane.
     * @param point The point to check.
     * @return True if inside, false otherwise.
     */
    bool containsPoint(const glm::vec3& point) const;

private:
    float _radius;
    float _shadowOffset;
    glm::vec4 _shadowColor;
    Eng::ShadowPlaneType _type;

    // Helper to compute infinite plane shadow matrix
    glm::mat4 computeInfiniteShadowMatrix(glm::vec4 lightPos) const;
};