#include "camera.h"
#include <GL/freeglut.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

Camera::Camera() : Node(),  _projectionMatrix(glm::mat4(1.0f))
{
	
}

Camera::~Camera()
{
}

void ENG_API Camera::setPerspective(float fov, float nearPlane, float farPlane)
{
	this->_projectionMatrix = glm::perspective(fov, _aspect, nearPlane, farPlane);
}

void ENG_API Camera::setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
	this->_projectionMatrix = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
}

glm::mat4 ENG_API Camera::computeInverse(void)
{
	return glm::inverse(this->getWC());
}

void ENG_API Camera::setAspect(float aspect) {
	_aspect = aspect;

	if (_config.type == Eng::CameraType::PERSPECTIVE)
		this->setPerspective(_config.fov, _config.nearPlane, _config.farPlane);
}

void Camera::setConfig(Eng::CameraConfig config) {
	if (config.type == Eng::CameraType::ORTHOGRAPHIC) {
		this->setOrthographic(config.left, config.right, config.bottom, config.top, config.nearPlane, config.farPlane);
	}
	else {
		this->setPerspective(config.fov, config.nearPlane, config.farPlane);
	}
	_config = config;
}

void ENG_API Camera::render(glm::mat4 cameraInverse)
{
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(this->_projectionMatrix));
	//glMatrixMode(GL_MODELVIEW);
	//glLoadMatrixf(glm::value_ptr(this->computeInverse()));
}
