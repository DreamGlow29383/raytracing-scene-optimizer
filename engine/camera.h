/**
* @file camera.h
* @brief Camera node implementation for scene viewing.
*/

#pragma once

#include "node.h"

/**
* @class Camera
* @brief Represents a camera in the scene graph.
* * Takes care of projection matrices (Perspective/Orthographic) and
* view matrices based on its position in the world.
*/
class ENG_API Camera : public Node
{
public:
	Camera();
	~Camera();
	
	/**
	 * @brief Computes and returns the inverse of the camera's world transformation matrix.
	 * @param void
	 * @return The inverse world transformation matrix of the camera.
	 */
	glm::mat4 computeInverse(void);

	/**
	 * @brief Sets the aspect ratio for the camera's projection.
	 * @param aspect The new aspect ratio (width / height).
	 */
	void setAspect(float aspect);

	/**
	 * @brief Applies a configuration structure to the camera.
	 * @param config The configuration struct containing FOV, planes, etc.
	 */
	void setConfig(Eng::CameraConfig config);

	/**
	 * @brief Renders the scene from the camera's perspective.
	 * @param cameraInverse The inverse of the camera's world transformation matrix.
	 */
	virtual void render(glm::mat4 cameraInverse) override;

	Eng::CameraConfig getConfig() const {
		return _config;
	}

	glm::mat4 getProj() const {
		return _projectionMatrix;
	}

private:
	glm::mat4 _projectionMatrix;
	float _aspect;
	Eng::CameraConfig _config;

	void setPerspective(float fov, float nearPlane, float farPlane);
	void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
};
