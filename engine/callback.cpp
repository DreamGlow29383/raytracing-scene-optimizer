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

#include <imgui.h>
#include <ImGuiFileDialog.h>
#include <backends/imgui_impl_glut.h>
#include <backends/imgui_impl_opengl3.h>

int windowId = NULL;
int _width;
int _height;

bool initialized = false;
bool wireFrameMode = false;

static auto lastRayTime = std::chrono::steady_clock::now();

static long long totalTime = 0;
static int rayCount = 0;

// FPS
std::chrono::steady_clock::time_point fpsLastTime = std::chrono::steady_clock::now();
int frameCount = 0;
float fps = 0;

void calculateFPS();
void printNodeHierarchy(Node* node, const std::string& prefix = "", bool isLast = false, bool isRoot = true);
float getDeltaTime();

void DrawMenuBar() {
	Eng::Base& eng = Eng::Base::getInstance();

	if (ImGui::BeginMainMenuBar()) {
		// --- File Menu ---
		if (ImGui::BeginMenu("File")) {
			IGFD::FileDialogConfig config; config.path = ".";

			if (ImGui::MenuItem("Generate")) {
				ImGuiFileDialog::Instance()->OpenDialog(
					"ChooseFileDlgKey",
					"Select a File to Generate",
					".obj,.gltf,.glb,.fbx",
					config
				);
			}

			if (ImGui::MenuItem("Import")) {
				ImGuiFileDialog::Instance()->OpenDialog(
					"ImportFileDlgKey",
					"Import Octree-Optimized Model",
					".oct",
					config
				);
			}

			if (ImGui::MenuItem("Export")) {
				IGFD::FileDialogConfig config;
				config.path = ".";
				config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;
				ImGuiFileDialog::Instance()->OpenDialog(
					"ExportFileDlgKey",           
					"Export Octree File",         
					".oct",                       
					config
				);
			}
			ImGui::EndMenu();
		}


		// --- View Menu ---
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Node Boundaries", nullptr, eng.getShowNodeBoundaries())) {
				eng.setShowNodeBoundaries(!eng.getShowNodeBoundaries());
			}

			if (ImGui::BeginMenu("Coloring")) {
				if (ImGui::MenuItem("None", nullptr, eng.getColoringMode() == 0)) {
					eng.setColoringMode(0);
				}
				if (ImGui::MenuItem("Depth", nullptr, eng.getColoringMode() == 1)) {
					eng.setColoringMode(1);
				}
				if (ImGui::MenuItem("Faces", nullptr, eng.getColoringMode() == 2)) {
					eng.setColoringMode(2);
				}
				if (ImGui::MenuItem("Node", nullptr, eng.getColoringMode() == 3)) {
					eng.setColoringMode(3);
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Benchmark")) {
			if (ImGui::MenuItem("Singular Raycast", nullptr, eng.getBenchmarkMode() == 3)) {
				glm::vec3 origin(
					((std::rand() % 2000) - 1000) / 100.0f,
					((std::rand() % 2000) - 1000) / 100.0f,
					((std::rand() % 2000) - 1000) / 100.0f
				);
				glm::vec3 direction = glm::normalize(glm::vec3(0.0f) - origin);
				eng.setBenchmarkMode(3);
				eng.castRay(origin, direction, 100.0f);
			}
			if (ImGui::MenuItem("Start Optimized", nullptr, eng.getBenchmarkMode() == 1)) {
				eng.setBenchmarkMode(1);
			}
			if (ImGui::MenuItem("Start Brute Force", nullptr, eng.getBenchmarkMode() == 2)) {
				eng.setBenchmarkMode(2);
			}
			if (ImGui::MenuItem("Stop", nullptr, eng.getBenchmarkMode() == 0)) {
				eng.setBenchmarkMode(0);
			}
			ImGui::EndMenu();
		}

		std::string fpsText = "FPS: " + std::to_string((int)fps);
		ImVec2 textSize = ImGui::CalcTextSize(fpsText.c_str());

		float textX = _width - textSize.x - 10;

		ImGui::SetCursorPosX(textX);
		ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", fpsText.c_str());

		ImGui::EndMainMenuBar();

		ImVec2 maxSize = ImVec2(_width, _height);
		ImVec2 minSize = ImVec2(_width / 2, _height / 2);
		if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
				std::cout << "Generate selected: " << filePath << std::endl;
				if (eng.getCurrentScene()->getNrOfChildren() > 2)
					delete eng.getCurrentScene()->removeChild(2);
				eng.addNodeFromFile(filePath);
				eng.getCurrentScene()->computeRenderList();
				printNodeHierarchy(eng.getCurrentScene());
			}
			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGuiFileDialog::Instance()->Display("ImportFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
				std::cout << "Importing: " << filePath << std::endl;
				if (eng.getCurrentScene()->getNrOfChildren() > 2)
					delete eng.getCurrentScene()->removeChild(2);
				eng.importOctree(filePath);
				eng.getCurrentScene()->computeRenderList();
				printNodeHierarchy(eng.getCurrentScene());
			}
			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGuiFileDialog::Instance()->Display("ExportFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();

				if (filePath.substr(filePath.find_last_of(".") + 1) != "oct") {
					filePath += ".oct";
				}

				std::cout << "Exporting to: " << filePath << std::endl;
				eng.exportOctree(filePath);
			}
			ImGuiFileDialog::Instance()->Close();
		}
	}
}

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

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGLUT_NewFrame();
	ImGui::NewFrame();
	ImGuiIO& io = ImGui::GetIO();

	//////////////////////////
	// 3D Rendering:

	// Clear the screen:
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_CULL_FACE);

	glMatrixMode(GL_MODELVIEW);

	// Calculate FPS
	calculateFPS();

	currentScene->render(currentScene->getCurrentCamera()->computeInverse());
	
	//////////////////////////
	// ImGui Interface

	DrawMenuBar();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (eng.getBenchmarkMode() > 0 && eng.getBenchmarkMode() < 3) {
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRayTime).count();
		if (elapsed >= 10) {
			lastRayTime = now;
			glm::vec3 origin(
				((std::rand() % 2000) - 1000) / 100.0f,
				((std::rand() % 2000) - 1000) / 100.0f,
				((std::rand() % 2000) - 1000) / 100.0f
			);
			glm::vec3 direction = glm::normalize(glm::vec3(0.0f) - origin);
			auto start = std::chrono::high_resolution_clock::now();
			eng.castRay(origin, direction, 100.0f);
			auto end = std::chrono::high_resolution_clock::now();
			totalTime += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
			rayCount++;
		}
	}

	if (rayCount > 0 && rayCount % 300 == 0) {
		std::cout << "Average over " << rayCount << " rays: " << (totalTime / rayCount) << " μs/ray" << std::endl;
		totalTime = 0;
		rayCount = 0;
	}

	glutSwapBuffers();
	glutPostWindowRedisplay(windowId);
}

void reshapeCallback(int width, int height)
{
	ImGui_ImplGLUT_ReshapeFunc(width, height);

	_width = width;
	_height = height;

	Eng::Base& eng = Eng::Base::getInstance();
	Scene* currentScene = eng.getCurrentScene();

	currentScene->getCurrentCamera()->setAspect((float) width / height);

	glViewport(0, 0, width, height);
}

void keyboardCallback(unsigned char key, int mouseX, int mouseY)
{
	ImGui_ImplGLUT_KeyboardFunc(key, mouseX, mouseY);

	Eng::Base& eng = Eng::Base::getInstance();
	Scene* currentScene = eng.getCurrentScene();
	currentScene->fireKeyPressedEvents(key);

	glutPostWindowRedisplay(windowId);
}

void keyboardUpCallback(unsigned char key, int mouseX, int mouseY) 
{
	ImGui_ImplGLUT_KeyboardUpFunc(key, mouseX, mouseY);

	Eng::Base& eng = Eng::Base::getInstance();
	Scene* currentScene = eng.getCurrentScene();
	currentScene->fireKeyReleasedEvents(key);

	glutPostWindowRedisplay(windowId);
}

void mouseCallback(int button, int state, int x, int y) {
	ImGui_ImplGLUT_MouseFunc(button, state, x, y);

	/*
	Eng::Base& eng = Eng::Base::getInstance();
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		std::cout << "Mouse (left) clicked at: " << x << ", " << y << std::endl;
		eng.castRay(x, y);
		glutPostWindowRedisplay(windowId);
	}
	*/
}

void specialCallback(int key, int mouseX, int mouseY)
{
	ImGui_ImplGLUT_SpecialFunc(key, mouseX, mouseY);

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

void specialUpCallback(int key, int mouseX, int mouseY) 
{
	ImGui_ImplGLUT_SpecialUpFunc(key, mouseX, mouseY);
}

void mouseWheelCallback(int wheel, int direction, int x, int y)
{
	ImGui_ImplGLUT_MouseWheelFunc(wheel, direction, x, y);
}

void motionCallback(int x, int y)
{
	ImGui_ImplGLUT_MotionFunc(x, y);
}

void passiveMotionCallback(int x, int y)
{
	ImGui_ImplGLUT_MotionFunc(x, y);
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