#include "raycast.h"
#include "engine.h"
#include "camera.h"
#include "scene.h"
#include <GL/freeglut.h>

void castRay(int mouseX, int mouseY) {
	Eng::Base& eng = Eng::Base::getInstance();
	Scene* scene = eng.getCurrentScene();
	Camera* camera = scene->getCurrentCamera();
	Eng::CameraConfig cameraConfig = camera->getConfig();

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble projMatrix[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

	glm::mat4 cameraWorld = camera->getWC();
	glm::mat4 viewMatrix = glm::inverse(cameraWorld);

	// Convert to GLdouble array for gluUnProject
	GLdouble modelMatrix[16];
	for (int i = 0; i < 16; i++) {
		modelMatrix[i] = viewMatrix[i / 4][i % 4];
	}

	// Convert screen coordinates to gldouble
	GLdouble winX = (GLdouble)mouseX;
	GLdouble winY = (GLdouble)viewport[3] - (GLdouble)mouseY;

	// Ray start point at near plane
	GLdouble nearX, nearY, nearZ;
	gluUnProject(winX, winY, 0.0, modelMatrix, projMatrix, viewport, &nearX, &nearY, &nearZ);

	// Ray end point at far plane
	GLdouble farX, farY, farZ;
	gluUnProject(winX, winY, 1.0, modelMatrix, projMatrix, viewport, &farX, &farY, &farZ);

	glm::vec3 rayStart = glm::vec3((GLfloat)nearX, (GLfloat)nearY, (GLfloat)nearZ);
	glm::vec3 rayEnd = glm::vec3((GLfloat)farX, (GLfloat)farY, (GLfloat)farZ);

	scene->setRay(rayStart, rayEnd);
}