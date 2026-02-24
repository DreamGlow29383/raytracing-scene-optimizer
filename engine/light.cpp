#include "light.h"

#include <GL/freeglut.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

int Light::lightCount = 0;

ENG_API Light::Light() : Node()
{
	_lightNum = lightCount;
	lightCount++;

	m_castShadow = false;

	glEnable(0x4000 + _lightNum);
}

ENG_API Light::~Light()
{
}

ENG_API void Light::setType(Eng::LightType type)
{
	m_type = type;
}

ENG_API void Light::setAttenuation(float constant, float linear, float quadratic)
{
	m_constant = constant;
	m_linear = linear;
	m_quadratic = quadratic;
}

ENG_API void Light::setCutOff(float cutOff)
{
	m_cutOff = cutOff;;
}

ENG_API void Light::setAmbient(const glm::vec3& ambient)
{
	m_ambient = ambient;
}

ENG_API void Light::setDiffuse(const glm::vec3& diffuse)
{
	m_diffuse = diffuse;
}

ENG_API void Light::setSpecular(const glm::vec3& specular)
{
	m_specular = specular;
}

ENG_API void Light::setIntensity(float intensity)
{
	m_intensity = intensity;
}

void Light::setCastShadow(bool castShadow)
{
	m_castShadow = castShadow;
}

bool Light::getCastShadow() const
{
	return m_castShadow;
}

ENG_API void Light::render(glm::mat4 cameraInverse)
{
	if (lightCount >= 8) {
		std::cout << "[WARNING] Maximum number of lights reached (8)." << std::endl;
		return;
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(glm::mat4(1.0f)));

	glm::vec4 amb = glm::vec4(m_ambient * m_intensity, 1.0f);
	glm::vec4 diff = glm::vec4(m_diffuse * m_intensity, 1.0f);
	glm::vec4 spec = glm::vec4(m_specular * m_intensity, 1.0f);
	
	int lightId = 0x4000 + _lightNum;
	glLightfv(lightId, GL_AMBIENT, glm::value_ptr(amb));
	glLightfv(lightId, GL_DIFFUSE, glm::value_ptr(diff));
	glLightfv(lightId, GL_SPECULAR, glm::value_ptr(spec));

	glm::mat4 globalMatrix = cameraInverse * this->getWC();
	glm::vec3 position = glm::vec3(globalMatrix[3]);

	if (m_type == Eng::DIRECTIONAL)
	{
		glm::vec3 dir = glm::mat3(globalMatrix) * glm::vec3(0.0f, 0.0f, 1.0f);
		
		GLfloat direction[] = { dir.x, dir.y, dir.z, 0.0f };
		glLightfv(lightId, GL_POSITION, direction);
	}
	else
	{
		GLfloat pos[] = { position.x, position.y, position.z, 1.0f };
		glLightfv(lightId, GL_POSITION, pos);

		glLightf(lightId, GL_CONSTANT_ATTENUATION, m_constant);
		glLightf(lightId, GL_LINEAR_ATTENUATION, m_linear);
		glLightf(lightId, GL_QUADRATIC_ATTENUATION, m_quadratic);
	}

	if (m_type == Eng::SPOT)
	{
		glm::vec3 dir = glm::mat3(globalMatrix) * glm::vec3(0.0f, 0.0f, 1.0f);
		glLightfv(lightId, GL_SPOT_DIRECTION, glm::value_ptr(dir));
		
		glLightf(lightId, GL_SPOT_CUTOFF, m_cutOff);
		glLightf(lightId, GL_SPOT_EXPONENT, 2.0f);
	}
	else 
	{
		glLightf(lightId, GL_SPOT_CUTOFF, 180.0f);
	}
}