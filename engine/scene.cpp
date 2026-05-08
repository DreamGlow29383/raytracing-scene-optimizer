#include "scene.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "light.h"
#include "mesh.h"

#include <iostream>
#include <algorithm>

Scene::Scene() {
	_nodes = std::map<int, Node*>();
}

Scene::~Scene() {
    for (FrameEvent* e : _frameEvents)
        delete e;
    
    for (auto& pair : _keyEvents)
        for (KeyEvent* e : pair.second)
            delete e;

    for (const auto& nodePair : _nodes)
        if (nodePair.second->getParent() == this)
            delete nodePair.second;
}

void Scene::setAmbient(glm::vec4 ambient) {
	_ambient = ambient;
}

Node* Scene::getNode(int id) {
	if (id == getId()) {
		return this;
	}

	if (_nodes[id] == NULL)
		std::cout << "ERROR: No node with ID: " << id << " found" << std::endl;

	return _nodes[id];
}

void Scene::updateMap(Node* node) {
	_nodes[node->getId()] = node;
}

void Scene::computeRenderList() {
    _renderList.clear();
    pass(this);

    glm::vec3 cameraPos = _currentCamera->getWC()[3];

    std::sort(_renderList.begin(), _renderList.end(),
        [cameraPos](Node* a, Node* b) {
            Light* lightA = dynamic_cast<Light*>(a);
            Light* lightB = dynamic_cast<Light*>(b);
            Mesh* meshA = dynamic_cast<Mesh*>(a);
            Mesh* meshB = dynamic_cast<Mesh*>(b);

            // Priority: Lights (1) > Meshes (2) > ShadowPlanes (3)
            int priorityA = lightA ? 1 : (meshA ? 2 : 3);
            int priorityB = lightB ? 1 : (meshB ? 2 : 3);

            if (priorityA != priorityB) {
                return priorityA < priorityB;
            }

            if (meshA && meshB) {
                // Sort meshes by distance (front-to-back)
                glm::vec3 posA = a->getWC()[3];
                glm::vec3 posB = b->getWC()[3];
                return glm::distance(posA, cameraPos) < glm::distance(posB, cameraPos);
            }

            return false;
        });
}

std::vector<Node*> Scene::getRenderList() {
    return _renderList;
}

void Scene::pass(Node* node) {
    Light* light = dynamic_cast<Light*>(node);
    Mesh* mesh = dynamic_cast<Mesh*>(node);

    if (light != nullptr || mesh != nullptr) {
        _renderList.push_back(node);
    }

    for (int i = 0; i < node->getNrOfChildren(); i++) {
        pass(node->getChild(i));
    }
}

void Scene::bindEvent(FrameEvent* event) {
	_frameEvents.push_back(event);
}

void Scene::bindEvent(char key, KeyEvent* event) {
	_keyEvents[key].push_back(event);
}

void Scene::fireFrameEvents(float deltaTime) {
	for (int i = 0; i < _frameEvents.size(); i++)
		_frameEvents.at(i)->invoke(deltaTime);
}

void Scene::fireKeyPressedEvents(char key) {
    if (_keyEvents.count(key) > 0) {
        std::vector<KeyEvent*>& events = _keyEvents[key];
        for (auto& event : events) {
            if (event) {
                event->invoke(true);
            }
        }
    }
}

void Scene::fireKeyReleasedEvents(char key) {
    if (_keyEvents.count(key) > 0) {
        std::vector<KeyEvent*>& events = _keyEvents[key];
        for (auto& event : events) {
            if (event) {
                event->invoke(false);
            }
        }
    }
}

void Scene::render(glm::mat4 cameraInverse) {
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, glm::value_ptr(_ambient));

    _currentCamera->render(cameraInverse);

    for (Node* node : _renderList) {
        node->render(cameraInverse);
    }

    if (_showRay) {
       glDisable(GL_LIGHTING);
       glLineWidth(1.0f);
       glBegin(GL_LINES);
       glColor3f(1.0f, 1.0f, 0.0f);
          glVertex3f(_rayStart.x, _rayStart.y, _rayStart.z);
          glVertex3f(_rayEnd.x, _rayEnd.y, _rayEnd.z);
       glEnd();
       glEnable(GL_LIGHTING);
    }
}

Camera* Scene::getCurrentCamera() {
	return _currentCamera;
}

void Scene::setCurrentCamera(Camera* camera) {
	_currentCamera = camera;
    computeRenderList();
}

void Scene::addText(std::string text) {
    _textLines.push_back(text);
}

std::vector<std::string> Scene::getTextLines() {
    return _textLines;
}