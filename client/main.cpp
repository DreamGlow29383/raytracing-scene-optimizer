/**
 * @file		main.cpp
 * @brief	Client application (that uses the graphics engine)
 *
 * @author	Ruben Barros (C) SUPSI [ruben.barros@supsi.ch]
 * @author	Lorenzo Vanina (C) SUPSI [lorenzo.vanina@supsi.ch]
 * @author	Davide Villa (C) SUPSI [davide.villa@supsi.ch]
 */

#include "engine.h"
#include <iostream>
#include <map>
#include <algorithm>

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

void moveCameraEvent(int nodeId, float deltaTime, glm::mat4 nodeTransform);
void cameraLeftEvent(int nodeId, bool keyDown, glm::mat4 nodeTransform);
void cameraRightEvent(int nodeId, bool keyDown, glm::mat4 nodeTransform);
void cameraUpEvent(int nodeId, bool keyDown, glm::mat4 nodeTransform);
void cameraDownEvent(int nodeId, bool keyDown, glm::mat4 nodeTransform);

std::map<char, bool> cameraKeys;
int selectedCamera = 0;

/**
 * Application entry point.
 * @param argc number of command-line arguments passed
 * @param argv array containing up to argc passed arguments
 * @return error code (0 on success, error code otherwise)
 */
int main(int argc, char *argv[])
{
	std::cout << "Hanoi Tower Project - Group 12" << std::endl;
	std::cout << std::endl;

	Eng::Base &eng = Eng::Base::getInstance();

	eng.init(argc, argv);

	int id = eng.createScene();
	eng.setCurrentScene(id);
	eng.setSceneAmbient(1.0f, 1.0f, 1.0f, 1.0f);

	Eng::CameraConfig cameraConfig;
	cameraConfig.type = Eng::CameraType::PERSPECTIVE;
	cameraConfig.fov = glm::radians(45.0f);
	cameraConfig.nearPlane = 1.0f;
	cameraConfig.farPlane = 100.0f;
	int cameraId = eng.addNodeCamera(cameraConfig);
	glm::mat4 cameraPos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 5.0f));
	glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	eng.setNodeTransform(cameraId, cameraPos * rotationY * rotationX);
	eng.setSceneCamera(cameraId);
	selectedCamera = cameraId;

	Eng::LightConfig lightConfig;
	lightConfig.type = Eng::LightType::POINT;
	lightConfig.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	lightConfig.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	lightConfig.specular = glm::vec3(1.0f, 1.0f, 1.0f);
	lightConfig.intensity = 1.0f;
	lightConfig.attenuation = glm::vec3(1.0f, 0.045f, 0.0075f);
	int lightId = eng.addNodeLight(lightConfig);
	glm::mat4 lightPos = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f));
	eng.setNodeTransform(lightId, lightPos);

	eng.addNodeFromFile("models/suzanne.glb");

	eng.bindSceneEvent(3, moveCameraEvent);
	eng.bindSceneEvent('w', 3, cameraUpEvent);
	eng.bindSceneEvent('a', 3, cameraLeftEvent);
	eng.bindSceneEvent('s', 3, cameraDownEvent);
	eng.bindSceneEvent('d', 3, cameraRightEvent);

	eng.run();

	eng.free();

	std::cout << "\n[application terminated]" << std::endl;
	return 0;
}

void moveCameraEvent(int nodeId, float deltaTime, glm::mat4 nodeTransform) {
	Eng::Base& eng = Eng::Base::getInstance();

	static float rotationSpeed = 45.0f;
	static float movementSpeed = 5.0f;

	glm::mat4 currentTransform = eng.getNodeTransform(selectedCamera);
	glm::vec3 cameraPos = glm::vec3(currentTransform[3]);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 forward = -glm::vec3(currentTransform[2]);
	glm::vec3 right = glm::vec3(currentTransform[0]);
	glm::vec3 up = glm::vec3(currentTransform[1]);

	float currentDistance = glm::length(cameraPos - cameraTarget);

	if (cameraKeys['a']) {
		float angle = -rotationSpeed * deltaTime;
		glm::vec3 toCamera = cameraPos - cameraTarget;
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 newToCamera = glm::vec3(rotation * glm::vec4(toCamera, 1.0f));
		cameraPos = cameraTarget + newToCamera;
	}

	if (cameraKeys['d']) {
		float angle = rotationSpeed * deltaTime;
		glm::vec3 toCamera = cameraPos - cameraTarget;
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 newToCamera = glm::vec3(rotation * glm::vec4(toCamera, 1.0f));
		cameraPos = cameraTarget + newToCamera;
	}

	if (cameraKeys['w']) {
		glm::vec3 toTarget = cameraTarget - cameraPos;
		float distance = glm::length(toTarget);
		if (distance > 2.0f) {
			glm::vec3 direction = glm::normalize(toTarget);
			cameraPos += direction * movementSpeed * deltaTime;
		}
	}

	if (cameraKeys['s']) {
		glm::vec3 toTarget = cameraTarget - cameraPos;
		glm::vec3 direction = glm::normalize(toTarget);
		cameraPos -= direction * movementSpeed * deltaTime;
	}

	glm::quat cameraRotation = glm::quatLookAt(glm::normalize(cameraTarget - cameraPos), cameraUp);
	currentTransform = glm::translate(glm::mat4(1.0f), cameraPos) * glm::mat4_cast(cameraRotation);

	eng.setNodeTransform(selectedCamera, currentTransform);
}

void cameraRightEvent(int nodeId, bool keyDown, glm::mat4 nodeTransform) {
	if (selectedCamera != 1)
		return;

	cameraKeys['d'] = keyDown;
}

void cameraLeftEvent(int nodeId, bool keyDown, glm::mat4 nodeTransform) {
	if (selectedCamera != 1)
		return;

	cameraKeys['a'] = keyDown;
}

void cameraUpEvent(int nodeId, bool keyDown, glm::mat4 nodeTransform) {
	if (selectedCamera != 1)
		return;

	cameraKeys['w'] = keyDown;
}

void cameraDownEvent(int nodeId, bool keyDown, glm::mat4 nodeTransform) {
	if (selectedCamera != 1)
		return;

	cameraKeys['s'] = keyDown;
}