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
#include <fstream>
#include <utility>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <imgui.h>
#include <backends/imgui_impl_glut.h>
#include <backends/imgui_impl_opengl3.h>

#ifdef _MSC_VER
    #include <stdlib.h>
    #define be32toh(x) _byteswap_ulong(x)
    #define be64toh(x) _byteswap_uint64(x)
#elif defined(__GNUC__) || defined(__clang__)
    #include <byteswap.h>
    #define be32toh(x) __bswap_32(x)
    #define be64toh(x) __bswap_64(x)
#endif

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

    glutInit(&argc, argv);
    #ifdef __FREEGLUT_EXT_H__
        glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    #endif
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);

    int window_w = 800;
    int window_h = 600;

    int pos_x = (glutGet(GLUT_SCREEN_WIDTH) / 2) - (window_w / 2);
    int pos_y = (glutGet(GLUT_SCREEN_HEIGHT) / 2) - (window_h / 2);

    glutInitWindowSize(window_w, window_h);
    glutInitWindowPosition(pos_x, pos_y);

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    windowId = glutCreateWindow("Hanoi Tower - Group 12");

    glewExperimental = GL_TRUE;
    glewInit();
    glGetError();

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // <-- forces crash to happen at the bad call
    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar* message, const void* userParam)
        {
            if (severity == GL_DEBUG_SEVERITY_HIGH) {
                std::cerr << "[GL ERROR] " << message << std::endl;
            }
        }, nullptr);

    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialCallback);
    glutSpecialUpFunc(specialUpCallback);
    glutMouseFunc(mouseCallback);
    glutMouseWheelFunc(mouseWheelCallback);
    glutMotionFunc(motionCallback);
    glutPassiveMotionFunc(passiveMotionCallback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

    // --- IMGUI INIT ---

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL3_Init("#version 440");

    // ---

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();

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
    std::vector<Vertex*> outVertices;
    std::vector<Face*> outFaces;

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

std::vector<std::pair<uint64_t, OctreeNode*>> exportedNodes;
void traverseOctree(uint64_t completeId, OctreeNode* node) {
    if (!node->isSplit())
        exportedNodes.push_back({ completeId, node });

    if (node->hasChildren()) {
        for (OctreeNode* child : node->getChildren()) {
            uint64_t newId = completeId + (node->getId() >> (3 * node->getDepth()));
            traverseOctree(newId, child);
        }
    }
}

void ENG_API Eng::Base::exportOctree(const std::string& outfilepath) {
    std::ofstream file(outfilepath, std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + outfilepath);
    }

    const uint32_t magic = be32toh(0x4F435452); // "OCTR" in hex
    file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));

    const uint8_t version = 1;
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));

    const uint8_t max_depth = 10;
    file.write(reinterpret_cast<const char*>(&max_depth), sizeof(max_depth));

    // Write Octree Data
    Scene* scene = this->getCurrentScene();
    Mesh* mesh = dynamic_cast<Mesh*>(scene->getChild(2));

    if (mesh) {
        OctreeNode* root = mesh->getOctreeRoot();

        exportedNodes.clear();
        traverseOctree(root->getId(), root);

        for (std::pair<uint64_t, OctreeNode*> pair : exportedNodes) {
            uint64_t id = be64toh(pair.first);
            OctreeNode* node = pair.second;
            uint8_t node_depth = node->getDepth();
            std::vector<Face*> faces = node->getFaces();
            uint32_t n_faces = be32toh(faces.size());
            
            file.write(reinterpret_cast<const char*>(&node_depth), sizeof(node_depth));
            file.write(reinterpret_cast<const char*>(&id), sizeof(id));
            file.write(reinterpret_cast<const char*>(&n_faces), sizeof(n_faces));
            for (Face* face : faces) {
                std::vector<uint32_t> indices = face->_indices;
                std::cout << indices.size() << std::endl;
                for (uint32_t index : indices) {
                    uint32_t swp_index = be32toh(index);
                    file.write(reinterpret_cast<const char*>(&swp_index), sizeof(swp_index));
                }
            }
        }
    }
    else {
        throw std::runtime_error("No valid mesh found for octree export");
    }

    if (!file.good()) {
        throw std::runtime_error("Failed to write to file: " + outfilepath);
    }

    file.close();
}