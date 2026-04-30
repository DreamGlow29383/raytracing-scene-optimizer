#include "callback.h"

#include "engine.h"
#include "node.h"
#include "light.h"
#include "camera.h"
#include "mesh.h"
#include "scene.h"
#include "importer.h"

#include <chrono>
#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int windowId = NULL;
int _width;
int _height;

bool initialized = false;
bool wireFrameMode = false;

// FPS
std::chrono::steady_clock::time_point fpsLastTime = std::chrono::steady_clock::now();
int frameCount = 0;
float fps = 0;

void calculateFPS();
void printNodeHierarchy(Node* node, const std::string& prefix = "", bool isLast = false, bool isRoot = true);
float getDeltaTime();

void displayCallback() 
{
	//////////////////////////
	// Initial Setup:

	Eng::Base& eng = Eng::Base::getInstance();
	Scene* currentScene = eng.getCurrentScene();

	currentScene->fireFrameEvents(getDeltaTime());

	if (windowId == NULL)
		windowId = eng.getWindowId();
	
	if (!initialized)
	{
		initialized = true;

		std::cout << "[>] Current Scene Hierarchy: " << std::endl;
		printNodeHierarchy(currentScene);
	}

	//////////////////////////
	// 3D Rendering:

	// Clear the screen:
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Calculate FPS
	calculateFPS();

	currentScene->render(currentScene->getCurrentCamera()->computeInverse());

	//////////////////////////
    // 2D Text Rendering:

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(glm::mat4(1.0f)));
	gluOrtho2D(0, _width, _height, 0);  // Top-left origin

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(glm::mat4(1.0f)));

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glColor3f(1.0f, 1.0f, 1.0f);

	// FPS text
	std::string fpsStr = "FPS: " + std::to_string((int)fps);
	float fpsWidth = 0.0f;
	for (char c : fpsStr) {
		fpsWidth += glutBitmapWidth(GLUT_BITMAP_8_BY_13, c);
	}
	glRasterPos2f(_width - fpsWidth - 10.0f, 20.0f);
	for (char c : fpsStr) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
	}

	// Other text
	float verticalShift = 20.0f * (currentScene->getTextLines().size());

	for (const std::string& line : currentScene->getTextLines()) {
		float lineWidth = 0.0f;
		for (char c : line) {
			lineWidth += glutBitmapWidth(GLUT_BITMAP_8_BY_13, c);
		}

		float xPos = _width - lineWidth - 10.0f;
		float yPos = _height - verticalShift;

		glRasterPos2f(xPos, yPos);
		for (char c : line) {
			glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
		}

		verticalShift -= 20.0f;
	}

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);

	glutSwapBuffers();
	glutPostWindowRedisplay(windowId);
}

void reshapeCallback(int width, int height)
{
	_width = width;
	_height = height;

	Eng::Base& eng = Eng::Base::getInstance();
	Scene* currentScene = eng.getCurrentScene();

	currentScene->getCurrentCamera()->setAspect((float) width / height);

	glViewport(0, 0, width, height);
}

void keyboardCallback(unsigned char key, int mouseX, int mouseY)
{
	Eng::Base& eng = Eng::Base::getInstance();
	Scene* currentScene = eng.getCurrentScene();
	currentScene->fireKeyPressedEvents(key);

	glutPostWindowRedisplay(windowId);
}

void keyboardUpCallback(unsigned char key, int mouseX, int mouseY) {
	Eng::Base& eng = Eng::Base::getInstance();
	Scene* currentScene = eng.getCurrentScene();
	currentScene->fireKeyReleasedEvents(key);

	glutPostWindowRedisplay(windowId);
}

void mouseCallback(int button, int state, int x, int y) {
	Eng::Base& eng = Eng::Base::getInstance();
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		std::cout << "Mouse (left) clicked at: " << x << ", " << y << std::endl;
		eng.castRaySurface(x, y);
		glutPostWindowRedisplay(windowId);
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		std::cout << "Mouse (right) clicked at: " << x << ", " << y << std::endl;
		eng.castRayThrough(x, y);
		glutPostWindowRedisplay(windowId);
	}
}

void specialCallback(int key, int mouseX, int mouseY)
{
	switch (key)
	{
	case GLUT_KEY_CTRL_L:
		wireFrameMode = !wireFrameMode;
		if (wireFrameMode)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	}

	glutPostWindowRedisplay(windowId);
}

void calculateFPS() {
	frameCount++;

	auto now = std::chrono::steady_clock::now();
	float elapsed = std::chrono::duration<float>(now - fpsLastTime).count();

	if (elapsed >= 1.0f) {
		fps = frameCount / elapsed;
		frameCount = 0;
		fpsLastTime = now;
	}
}

void printNodeHierarchy(Node* node, const std::string& prefix, bool isLast, bool isRoot) {
	if (!node) return;

	std::cout << prefix;

	if (isRoot) {
		std::cout << "";
	}
	else {
		std::cout << (isLast ? "|__ " : "|-- ");
	}

	std::string nodeType = "Node";
	if (dynamic_cast<Mesh*>(node)) nodeType = "Mesh";
	else if (dynamic_cast<Light*>(node)) nodeType = "Light";
	else if (dynamic_cast<Camera*>(node)) nodeType = "Camera";

	std::cout << node->getId() << " : " << "[" << nodeType << "] " << node->getName() << std::endl;

	std::string childPrefix = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));

	int childCount = node->getNrOfChildren();
	for (int i = 0; i < childCount; i++) {
		bool childIsLast = (i == childCount - 1);
		printNodeHierarchy(node->getChild(i), childPrefix, childIsLast, false);
	}
}

float getDeltaTime() {
	static auto lastTime = std::chrono::steady_clock::now();
	auto now = std::chrono::steady_clock::now();

	float delta = std::chrono::duration<float>(now - lastTime).count();
	lastTime = now;

	return delta;
}