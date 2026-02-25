/**
 * @file light.h
 * @brief Light node implementation.
 */

#pragma once

#include "node.h"

#include <string>

 /**
  * @class Light
  * @brief Represents a light source in the scene (Point, Spot, or Directional).
  */
class ENG_API Light : public Node
{
public:
	Light();
	~Light();

	/**
	 * @brief Sets the type of the light.
	 * @param type One of DIRECTIONAL, POINT, or SPOT.
	 */
	void setType(Eng::LightType type);

	/**
	 * @brief Sets attenuation factors for point/spot lights.
	 * Formula: 1.0 / (constant + linear * dist + quadratic * dist^2)
	 * @param constant Constant factor.
	 * @param linear Linear factor.
	 * @param quadratic Quadratic factor.
	 */
	void setAttenuation(float constant, float linear, float quadratic);

	/**
	 * @brief Sets the cutoff angle for spot lights.
	 * @param cutOff Angle in degrees.
	 */
	void setCutOff(float cutOff);

	/**
	 * @brief Sets the ambient, diffuse, and specular colors of the light.
	 * @param ambient Ambient color.
	 * @param diffuse Diffuse color.
	 * @param specular Specular color.
	 */
	void setAmbient(const glm::vec3& ambient);
	void setDiffuse(const glm::vec3& diffuse);
	void setSpecular(const glm::vec3& specular);

	/**
	 * @brief Sets the overall intensity of the light.
	 * @param intensity Multiplier for the light color.
	 */
	void setIntensity(float intensity);

	/**
	 * @brief Renders the light (sets shader uniforms).
	 * @param cameraInverse The inverse of the camera's world transformation matrix.
	 */
	virtual void render(glm::mat4 cameraInverse) override;

private:
	static int lightCount;
	int _lightNum;
	Eng::LightType m_type;

	glm::vec3 m_ambient;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;
	float m_intensity;

	// Attenuation (for Point or Spot light)
	float m_constant;
	float m_linear;
	float m_quadratic;

	// Cutoff (only for Spot light)
	float m_cutOff;

	bool m_castShadow;
};
