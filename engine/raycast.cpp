#include "raycast.h"
#include "engine.h"
#include "camera.h"
#include "scene.h"
#include <GL/freeglut.h>
#include <iostream>

void castRay(int mouseX, int mouseY) {
	Eng::Base& eng = Eng::Base::getInstance();
	Scene* scene = eng.getCurrentScene();
	Camera* camera = scene->getCurrentCamera();
	Eng::CameraConfig cameraConfig = camera->getConfig();

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble projMatrix[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

	glm::mat4 view = camera->getTransform();

	glm::mat4 cameraWorld = glm::inverse(view);
	glm::mat4 viewMatrix = glm::inverse(cameraWorld);

	// Convert to GLdouble array for gluUnProject
	GLdouble modelMatrix[16];
	for (int i = 0; i < 16; i++) {
		modelMatrix[i] = viewMatrix[i / 4][i % 4];
	}

	// Convert screen coordinates to gldouble
	GLdouble winX = (GLdouble)mouseX;
	GLdouble winY = (GLdouble)viewport[3] - (GLdouble)mouseY;
	GLfloat depth;
	glReadPixels(mouseX, mouseY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

	// Ray start point at near plane
	GLdouble nearX, nearY, nearZ;
	gluUnProject(winX, winY, 0.0f, modelMatrix, projMatrix, viewport, &nearX, &nearY, &nearZ);

	// Ray end point at far plane
	GLdouble farX, farY, farZ;
	gluUnProject(winX, winY, 1.0f, modelMatrix, projMatrix, viewport, &farX, &farY, &farZ);

	glm::vec3 rayStart = glm::vec3((GLfloat)nearX, (GLfloat)nearY, (GLfloat)nearZ);
	//glm::vec3 rayStart = glm::vec3((GLfloat)50, (GLfloat)50, (GLfloat)100);
	glm::vec3 rayEnd = glm::vec3((GLfloat)farX, (GLfloat)farY, (GLfloat)farZ);
	//glm::vec3 rayEnd = glm::vec3((GLfloat)-50, (GLfloat)50, (GLfloat)-100);

	std::cout << "rayStart: (" << rayStart.x << ", " << rayStart.y << ", " << rayStart.z << ")" << std::endl;
	std::cout << "rayEnd: (" << rayEnd.x << ", " << rayEnd.y << ", " << rayEnd.z << ")" << std::endl;

	scene->setRay(rayStart, rayEnd);
}