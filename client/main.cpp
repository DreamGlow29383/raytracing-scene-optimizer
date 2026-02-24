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

	eng.run();

	eng.free();

	std::cout << "\n[application terminated]" << std::endl;
	return 0;
}