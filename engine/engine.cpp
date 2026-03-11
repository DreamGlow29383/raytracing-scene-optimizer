#define FREEIMAGE_LIB
#include "engine.h"
#include "callback.h"
#include "node.h"
#include "scene.h"
#include "light.h"
#include "camera.h"
#include "importer.h"
#include "frame_event.h"

#include <iostream>   
#include <source_location>
#include <FreeImage.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

struct Eng::Base::Reserved
{
    bool initFlag;

    Reserved() : initFlag{ false } 
    {}
};


ENG_API Eng::Base::Base() : reserved(std::make_unique<Eng::Base::Reserved>())
{  
#ifdef _DEBUG   
    std::cout << "[+] " << std::source_location::current().function_name() << " invoked" << std::endl;
#endif
}

ENG_API Eng::Base::~Base()
{
#ifdef _DEBUG
    std::cout << "[-] " << std::source_location::current().function_name() << " invoked" << std::endl;
#endif
}

Eng::Base ENG_API &Eng::Base::getInstance()
{
    static Base instance;
    return instance;
}

bool ENG_API Eng::Base::init(int argc, char* argv[])
{
    std::cout 
    << "OGL Graphics Engine,\n"
    << "Ruben Barros Freitas,\n"
    << "Lorenzo Vanina,\n"
    << "Davide Villa.\n"
    << std::endl;

    FreeImage_Initialise();

    if (reserved->initFlag)
    {
        std::cout << "ERROR: engine already initialized" << std::endl;
        return false;
    }

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
    
    glutInit(&argc, argv);

    int window_w = 800;
    int window_h = 600;

    int pos_x = (glutGet(GLUT_SCREEN_WIDTH) / 2) - (window_w / 2);
    int pos_y = (glutGet(GLUT_SCREEN_HEIGHT) / 2) - (window_h / 2);

    glutInitWindowSize(window_w, window_h);
    glutInitWindowPosition(pos_x, pos_y);

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    windowId = glutCreateWindow("Hanoi Tower - Group 12");

    glewInit();

    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialCallback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

    std::cout << "[>] " << LIB_NAME << " initialized" << std::endl;
    reserved->initFlag = true;

    return true;
}

bool ENG_API Eng::Base::run() {
    std::cout << "[>] " << "Running main loop" << std::endl;

    this->getCurrentScene()->computeRenderList();
    glutMainLoop();

    return true;
}

bool ENG_API Eng::Base::free()
{
    if (!reserved->initFlag)
    {
        std::cout << "ERROR: engine not initialized" << std::endl;
        return false;
    }

    std::cout << "[<] " << LIB_NAME << " deinitialized" << std::endl;
    reserved->initFlag = false;

    for (const auto& scenePair : scenes) {
        delete(scenePair.second);
    }

	FreeImage_DeInitialise();

    return true;
}

int ENG_API Eng::Base::createScene()
{
    if (!reserved->initFlag)
    {
        std::cout << "ERROR: engine not initialized" << std::endl;
        return -1;
    }

    Scene* scene = new Scene();
    scenes[scene->getId()] = scene;

    if (currentSceneId == -1) {
        currentSceneId = scene->getId();
    }

    std::cout << "[+] Scene created with ID: " << scene->getId() << std::endl;
    return scene->getId();
}

void ENG_API Eng::Base::setCurrentScene(int sceneId)
{
    if (scenes[sceneId] != NULL) {
        currentSceneId = sceneId;
        std::cout << "[>] Current Scene set to: " << sceneId << std::endl;
    }
    else {
        std::cout << "ERROR: Scene " << sceneId << " not found" << std::endl;
    }
}

void recursiveSceneUpdate(Scene* scene, Node* node) {
    scene->updateMap(node);
    for (int i = 0; i < node->getNrOfChildren(); i++)
        recursiveSceneUpdate(scene, node->getChild(i));
}

void Eng::Base::addNode(Node* node)
{
    if (!node) {
        std::cout << "ERROR: Cannot add null node" << std::endl;
        return;
    }

    getCurrentScene()->addChild(node);
    recursiveSceneUpdate(getCurrentScene(), node);
}

void Eng::Base::addNodeTo(Node* parent, Node* node)
{
    if (!node) {
        std::cout << "ERROR: Cannot add null node" << std::endl;
        return;
    }

    parent->addChild(node);
    recursiveSceneUpdate(getCurrentScene(), node);
}

Scene* Eng::Base::getCurrentScene()
{
    if (currentSceneId == -1) {
        std::cout << "ERROR: No current scene set" << std::endl;
        return nullptr;
    }

    Scene* scene = scenes[currentSceneId];
    if (scene == NULL) {
        std::cout << "ERROR: Current scene " << currentSceneId << " not found" << std::endl;
        return nullptr;
    }

    return scene;
}

void ENG_API Eng::Base::setSceneAmbient(float r, float g, float b, float a) {
    Scene* current = this->getCurrentScene();
    current->setAmbient(glm::vec4(r, g, b, a));
}

void ENG_API Eng::Base::setSceneCamera(int id) {
    Node* node = this->getCurrentScene()->getNode(id);

    Camera* camera = dynamic_cast<Camera*>(node);
    if (camera != nullptr) {
        this->getCurrentScene()->setCurrentCamera(camera);
        std::cout << "[>] Current Scene Camera set to: " << id << std::endl;
    }
    else {
        std::cerr << "ERROR: Node with ID " << id << " is not a Camera!" << std::endl;
    }
}

int ENG_API Eng::Base::addNodeLight(int parent, Eng::LightConfig config) {
    Light* light = new Light();
    light->setType(config.type);
    light->setAttenuation(config.attenuation.x, config.attenuation.y, config.attenuation.z);
    light->setCutOff(config.cutoff);
    light->setAmbient(config.ambient);
    light->setDiffuse(config.diffuse);
    light->setSpecular(config.specular);
    light->setIntensity(config.intensity);

    addNodeTo(getCurrentScene()->getNode(parent), light);
    return light->getId();
}

int Eng::Base::addNodeLight(Eng::LightConfig config) {
    return addNodeLight(currentSceneId, config);
}

void Eng::Base::setNodeLightColor(int id, glm::vec3 color)
{
    Node* node = this->getCurrentScene()->getNode(id);
    Light* light = dynamic_cast<Light*>(node);

    if (light != nullptr) {
        light->setDiffuse(color);
    }
}

int ENG_API Eng::Base::addNodeCamera(int parent, Eng::CameraConfig config) {
    Camera* camera = new Camera();
    camera->setConfig(config);

    addNodeTo(getCurrentScene()->getNode(parent), camera);
    return camera->getId();
}

int ENG_API Eng::Base::addNodeCamera(Eng::CameraConfig config) {
    return addNodeCamera(currentSceneId, config);
}

int ENG_API Eng::Base::addNodeFromFile(int parent, const std::string& filepath) {
    bool success = importFile(filepath, outVertices, outFaces);

    if (success) {
        Mesh* mesh = new Mesh(outFaces, outVertices);
        mesh->setName("Mesh");
        std::cout << "[+] Mesh Loaded: " << mesh->getName() << std::endl;
        addNodeTo(getCurrentScene()->getNode(parent), mesh);
        return mesh->getId();
    }

    return NULL;
}

int ENG_API Eng::Base::addNodeFromFile(const std::string& filepath) {
    return addNodeFromFile(currentSceneId, filepath);
}

void ENG_API Eng::Base::addSceneText(std::string text) {
    this->getCurrentScene()->addText(text);
}

void ENG_API Eng::Base::setNodeTransform(int id, glm::mat4 transform) {
    getCurrentScene()->getNode(id)->setTransform(transform);
}

glm::mat4 ENG_API Eng::Base::getNodeTransform(int id) {
    return getCurrentScene()->getNode(id)->getTransform();
}

int ENG_API Eng::Base::getWindowId() 
{
    return windowId;
}

void ENG_API Eng::Base::bindSceneEvent(int nodeId, FrameCallback func) {
    Scene* scene = getCurrentScene();
    Node* target = scene->getNode(nodeId);

    FrameEvent* event = new FrameEvent(target, func);
    scene->bindEvent(event);
}

void ENG_API Eng::Base::bindSceneEvent(char key, int nodeId, KeyCallback func) {
    Scene* scene = getCurrentScene();
    Node* target = scene->getNode(nodeId);

    KeyEvent* event = new KeyEvent(target, func);
    scene->bindEvent(key, event);
}