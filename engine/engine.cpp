#define FREEIMAGE_LIB
#include "engine.h"
#include "callback.h"
#include "node.h"
#include "scene.h"
#include "light.h"
#include "camera.h"
#include "importer.h"
#include "frame_event.h"
#include "byte_util.h"
#include "face.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>

#include <chrono>
#include <thread>
#include <iostream>
#include <source_location>
#include <FreeImage.h>
#include <fstream>
#include <utility>
#include <bitset>
#include <chrono>
#include <filesystem>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <imgui.h>
#include <backends/imgui_impl_glut.h>
#include <backends/imgui_impl_opengl3.h>

#ifdef _MSC_VER
#include <stdlib.h>
#define be16toh(x) _byteswap_ushort(x)
#define be32toh(x) _byteswap_ulong(x)
#define be64toh(x) _byteswap_uint64(x)
#elif defined(__GNUC__) || defined(__clang__)
#include <byteswap.h>
#define be16toh(x) __bswap_16(x)
#define be32toh(x) __bswap_32(x)
#define be64toh(x) __bswap_64(x)
#endif

struct Eng::Base::Reserved
{
    bool initFlag;

    Reserved() : initFlag{false}
    {
    }
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

bool ENG_API Eng::Base::init(int argc, char *argv[])
{

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
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);

    int window_h, window_w;
#if defined(__linux__)
    window_w = 1600;
    window_h = 1200;
#else
    window_w = 800;
    window_h = 600;
#endif

    int pos_x = (glutGet(GLUT_SCREEN_WIDTH) / 2) - (window_w / 2);
    int pos_y = (glutGet(GLUT_SCREEN_HEIGHT) / 2) - (window_h / 2);

    glutInitWindowSize(window_w, window_h);
    glutInitWindowPosition(pos_x, pos_y);

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    windowId = glutCreateWindow("Raytracing Octree Optimization");

    glewExperimental = GL_TRUE;
    glewInit();
    glGetError();

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // <-- forces crash to happen at the bad call
    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
                              GLsizei length, const GLchar *message, const void *userParam)
                           {
            if (severity == GL_DEBUG_SEVERITY_HIGH) {
                std::cerr << "[GL ERROR] " << message << std::endl;
            } },
                           nullptr);

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

    glutSetOption(GLUT_MULTISAMPLE, 8);
    glEnable(GL_MULTISAMPLE); // enable anti-aliasing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

    // --- IMGUI INIT ---

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
#if defined(__linux__)
    io.FontGlobalScale = 2.0f;
#endif

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL3_Init("#version 440");

    // ---

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    std::cout << "[>] " << LIB_NAME << " initialized" << std::endl;
    reserved->initFlag = true;

    return true;
}

bool ENG_API Eng::Base::run()
{
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

    for (const auto &scenePair : scenes)
    {
        delete (scenePair.second);
    }
    scenes.clear();

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

    Scene *scene = new Scene();
    scenes[scene->getId()] = scene;

    if (currentSceneId == -1)
    {
        currentSceneId = scene->getId();
    }

    std::cout << "[+] Scene created with ID: " << scene->getId() << std::endl;
    return scene->getId();
}

void ENG_API Eng::Base::setCurrentScene(int sceneId)
{
    if (scenes[sceneId] != NULL)
    {
        currentSceneId = sceneId;
        std::cout << "[>] Current Scene set to: " << sceneId << std::endl;
    }
    else
    {
        std::cout << "ERROR: Scene " << sceneId << " not found" << std::endl;
    }
}

void recursiveSceneUpdate(Scene *scene, Node *node)
{
    scene->updateMap(node);
    for (int i = 0; i < node->getNrOfChildren(); i++)
        recursiveSceneUpdate(scene, node->getChild(i));
}

void Eng::Base::addNode(Node *node)
{
    if (!node)
    {
        std::cout << "ERROR: Cannot add null node" << std::endl;
        return;
    }

    getCurrentScene()->addChild(node);
    recursiveSceneUpdate(getCurrentScene(), node);
}

void Eng::Base::addNodeTo(Node *parent, Node *node)
{
    if (!node)
    {
        std::cout << "ERROR: Cannot add null node" << std::endl;
        return;
    }

    parent->addChild(node);
    recursiveSceneUpdate(getCurrentScene(), node);
}

Scene *Eng::Base::getCurrentScene()
{
    if (currentSceneId == -1)
    {
        std::cout << "ERROR: No current scene set" << std::endl;
        return nullptr;
    }

    Scene *scene = scenes[currentSceneId];
    if (scene == NULL)
    {
        std::cout << "ERROR: Current scene " << currentSceneId << " not found" << std::endl;
        return nullptr;
    }

    return scene;
}

void ENG_API Eng::Base::setSceneAmbient(float r, float g, float b, float a)
{
    Scene *current = this->getCurrentScene();
    current->setAmbient(glm::vec4(r, g, b, a));
}

void ENG_API Eng::Base::setSceneCamera(int id)
{
    Node *node = this->getCurrentScene()->getNode(id);

    Camera *camera = dynamic_cast<Camera *>(node);
    if (camera != nullptr)
    {
        this->getCurrentScene()->setCurrentCamera(camera);
        std::cout << "[>] Current Scene Camera set to: " << id << std::endl;
    }
    else
    {
        std::cerr << "ERROR: Node with ID " << id << " is not a Camera!" << std::endl;
    }
}

int ENG_API Eng::Base::addNodeLight(int parent, Eng::LightConfig config)
{
    Light *light = new Light();
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

int Eng::Base::addNodeLight(Eng::LightConfig config)
{
    return addNodeLight(currentSceneId, config);
}

void Eng::Base::setNodeLightColor(int id, glm::vec3 color)
{
    Node *node = this->getCurrentScene()->getNode(id);
    Light *light = dynamic_cast<Light *>(node);

    if (light != nullptr)
    {
        light->setDiffuse(color);
    }
}

int ENG_API Eng::Base::addNodeCamera(int parent, Eng::CameraConfig config)
{
    Camera *camera = new Camera();
    camera->setConfig(config);

    addNodeTo(getCurrentScene()->getNode(parent), camera);
    return camera->getId();
}

int ENG_API Eng::Base::addNodeCamera(Eng::CameraConfig config)
{
    return addNodeCamera(currentSceneId, config);
}

void ENG_API Eng::Base::removeNode(int id)
{
    Scene *scene = getCurrentScene();
    for (int i = 0; i < scene->getNrOfChildren(); i++)
    {
        Node *current = scene->getChild(i);
        if (current->getId() == id)
        {
            delete scene->removeChild(i);
            break;
        }
    }
}

int ENG_API Eng::Base::addNodeFromFile(int parent, const std::string &filepath)
{
    std::vector<Vertex *> outVertices;
    std::vector<Face *> outFaces;

    auto t0 = std::chrono::high_resolution_clock::now();
    bool success = importFile(filepath, outVertices, outFaces);
    auto t1 = std::chrono::high_resolution_clock::now();

    std::string meshName = filepath.substr(filepath.find_last_of("\\") + 1);
    if (success)
    {
        Mesh *mesh = new Mesh(outFaces, outVertices);
        auto t2 = std::chrono::high_resolution_clock::now();

        mesh->setName(meshName);
        std::cout << "[+] Mesh Loaded: " << mesh->getName() << std::endl;
        std::cout << "[>] Faces: " << outFaces.size() << std::endl;

        auto meshLoad = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        auto total = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t0).count();
        std::cout << "[>] Octree generation took: " << total << " ms\n";
        std::cout << "[>] Octree generation took (excluding mesh load): " << total - meshLoad << " ms\n";

        addNodeTo(getCurrentScene()->getNode(parent), mesh);
        return mesh->getId();
    }
    return NULL;
}

int ENG_API Eng::Base::addNodeFromFile(const std::string &filepath)
{
    return addNodeFromFile(currentSceneId, filepath);
}

void ENG_API Eng::Base::addSceneText(std::string text)
{
    this->getCurrentScene()->addText(text);
}

void ENG_API Eng::Base::setNodeTransform(int id, glm::mat4 transform)
{
    getCurrentScene()->getNode(id)->setTransform(transform);
}

glm::mat4 ENG_API Eng::Base::getNodeTransform(int id)
{
    return getCurrentScene()->getNode(id)->getTransform();
}

int ENG_API Eng::Base::getWindowId()
{
    return windowId;
}

void ENG_API Eng::Base::bindSceneEvent(int nodeId, FrameCallback func)
{
    Scene *scene = getCurrentScene();
    Node *target = scene->getNode(nodeId);

    FrameEvent *event = new FrameEvent(target, func);
    scene->bindEvent(event);
}

void ENG_API Eng::Base::bindSceneEvent(char key, int nodeId, KeyCallback func)
{
    Scene *scene = getCurrentScene();
    Node *target = scene->getNode(nodeId);

    KeyEvent *event = new KeyEvent(target, func);
    scene->bindEvent(key, event);
}

void ENG_API Eng::Base::setColoringMode(int mode)
{
    coloring_mode = mode;

    Scene *scene = this->getCurrentScene();
    Mesh *mesh = dynamic_cast<Mesh *>(scene->getChild(2));

    mesh->updateColorVBO(coloring_mode);
}

void ENG_API Eng::Base::setBenchmarkMode(int mode)
{
    benchmark_mode = mode;

    Scene *scene = this->getCurrentScene();
    scene->setShowRay(mode > 0);
}

std::vector<std::pair<uint64_t, OctreeNode *>> exportedNodes;
void traverseOctree(uint64_t completeId, OctreeNode *node)
{
    if (!node->hasChildren())
        exportedNodes.push_back({completeId, node});

    if (node->hasChildren())
    {
        for (OctreeNode *child : node->getChildren())
        {
            uint64_t newId = (completeId << 3) | child->getId();
            traverseOctree(newId, child);
        }
    }
}

void ENG_API Eng::Base::exportOctree(const std::string &outfilepath)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::ofstream file(outfilepath, std::ios::binary | std::ios::trunc);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + outfilepath);
    }

    std::vector<std::byte> fileData;

    const uint32_t magic = be32toh(0x4F435452); // "OCTR" in hex
    pushBytes(fileData, magic);

    const uint8_t version = 1;
    pushBytes(fileData, version);

    const uint8_t max_depth = OctreeNode::MAX_DEPTH;
    pushBytes(fileData, max_depth);

    Scene *scene = this->getCurrentScene();
    Mesh *mesh = dynamic_cast<Mesh *>(scene->getChild(2));
    std::vector<Face *> mesh_faces = mesh->getFaces();

    std::string meshName = mesh ? mesh->getName() : "Unknown";
    uint8_t nameLength = static_cast<uint8_t>(meshName.length());

    pushBytes(fileData, nameLength);
    pushString(fileData, meshName);

    const uint8_t padding = (4 - (meshName.length() % 4)) % 4;
    for (uint8_t i = 0; i < padding; ++i)
    {
        const char zero = 0;
        pushBytes(fileData, zero);
    }

    // Write Octree Data
    if (mesh)
    {
        OctreeNode *root = mesh->getOctreeRoot();

        exportedNodes.clear();

        traverseOctree(root->getId(), root);

        for (std::pair<uint64_t, OctreeNode *> pair : exportedNodes)
        {
            uint64_t id = be64toh(pair.first);
            OctreeNode *node = pair.second;
            uint8_t node_depth = node->getDepth();
            std::vector<Face *> faces = node->getFaces();
            uint32_t n_faces = be32toh(faces.size());

            // std::cout << "Node ID: " << std::bitset<64>(pair.first) << std::endl;
            // std::cout << "Depth: " << (int)node_depth << std::endl;
            // std::cout << "Faces: " << faces.size() << std::endl;

            glm::vec3 upper = node->getUpperBounds();
            glm::vec3 lower = node->getLowerBounds();

            pushBytes(fileData, node_depth);
            pushBytes(fileData, id);
            pushBytes(fileData, n_faces);

            pushBytes(fileData, upper.x);
            pushBytes(fileData, upper.y);
            pushBytes(fileData, upper.z);

            pushBytes(fileData, lower.x);
            pushBytes(fileData, lower.y);
            pushBytes(fileData, lower.z);

            for (Face *face : faces)
            {
                uint32_t swp_index = be32toh(face->_id);

                pushBytes(fileData, swp_index);
            }
        }
    }
    else
    {
        throw std::runtime_error("No valid mesh found for octree export");
    }

    file.write(reinterpret_cast<const char *>(fileData.data()), fileData.size());

    if (!file.good())
    {
        throw std::runtime_error("Failed to write to file: " + outfilepath);
    }

    file.close();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "[>] Octree export took: " << duration.count() << " ms" << std::endl;
}

struct NodeData
{
    uint64_t id;
    uint8_t depth;
    glm::vec3 upperCorner;
    glm::vec3 lowerCorner;
    std::vector<uint32_t> faceIndices;
};

struct PairHash
{
    size_t operator()(const std::pair<uint64_t, uint8_t> &p) const
    {
        return std::hash<uint64_t>()(p.first) ^ ((size_t)p.second << 32);
    }
};

void ENG_API Eng::Base::importOctree(const std::string &infilepath)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::ifstream file(infilepath, std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + infilepath);

    std::vector<std::byte> fileData(std::filesystem::file_size(infilepath));
    file.read(reinterpret_cast<char *>(fileData.data()), fileData.size());
    file.close();

    size_t offset = 0;

    uint32_t magic;
    readBytes(fileData, offset, magic);
    magic = be32toh(magic);

    if (magic != 0x4F435452)
        throw std::runtime_error("Invalid file format: Wrong magic number");

    uint8_t version;
    readBytes(fileData, offset, version);

    if (version != 1)
        throw std::runtime_error("Unsupported file version: " + std::to_string(version));

    uint8_t max_depth;
    readBytes(fileData, offset, max_depth);

    std::cout << "Octree file info:" << std::endl;
    std::cout << "  Version: " << (int)version << std::endl;
    std::cout << "  Max Depth: " << (int)max_depth << std::endl;

    uint8_t nameLength;
    readBytes(fileData, offset, nameLength);

    std::string meshName;
    readString(fileData, offset, meshName, nameLength);

    uint8_t padding = (4 - (nameLength % 4)) % 4;
    offset += padding;

    std::cout << "  Mesh Name: " << meshName << std::endl;

    std::vector<NodeData> nodes;

    while (offset < fileData.size())
    {
        uint8_t node_depth;
        uint64_t node_id;
        uint32_t n_faces;
        float upperX, upperY, upperZ;
        float lowerX, lowerY, lowerZ;

        readBytes(fileData, offset, node_depth);
        readBytes(fileData, offset, node_id);
        readBytes(fileData, offset, n_faces);

        readBytes(fileData, offset, upperX);
        readBytes(fileData, offset, upperY);
        readBytes(fileData, offset, upperZ);

        readBytes(fileData, offset, lowerX);
        readBytes(fileData, offset, lowerY);
        readBytes(fileData, offset, lowerZ);

        node_id = be64toh(node_id);
        n_faces = be32toh(n_faces);

        std::vector<uint32_t> faceIndices;
        for (uint32_t f = 0; f < n_faces; ++f)
        {
            uint32_t index;
            readBytes(fileData, offset, index);
            faceIndices.push_back(be32toh(index));
        }

        NodeData data = {node_id, node_depth,
                         glm::vec3(upperX, upperY, upperZ),
                         glm::vec3(lowerX, lowerY, lowerZ),
                         faceIndices};
        nodes.push_back(data);
    }

    std::cout << "Total nodes loaded: " << nodes.size() << std::endl;

    size_t lastSlash = infilepath.find_last_of("\\");
    std::string directory = (lastSlash != std::string::npos)
                                ? infilepath.substr(0, lastSlash + 1)
                                : "";

    std::string meshpath = directory + meshName;

    std::vector<Vertex *> outVertices;
    std::vector<Face *> outFaces;
    auto t0 = std::chrono::high_resolution_clock::now();
    bool success = importFile(meshpath, outVertices, outFaces);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto meshImportTime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    std::unordered_map<std::pair<uint64_t, uint8_t>, OctreeNode *, PairHash> parentlessNodes;
    std::unordered_map<std::pair<uint64_t, uint8_t>, std::vector<OctreeNode *>, PairHash> parentBuffer;

    for (const NodeData &data : nodes)
    {
        uint8_t currentId = data.id & 0b111;
        OctreeNode *newNode = new OctreeNode(data.lowerCorner, data.upperCorner, data.depth, currentId);

        std::vector<Face *> nodeFaces;
        for (uint32_t face_id : data.faceIndices)
            nodeFaces.push_back(outFaces[face_id]);

        newNode->setFaces(nodeFaces);

        auto key = std::make_pair(data.id, data.depth);
        parentlessNodes.emplace(key, newNode);
    }

    OctreeNode *root = nullptr;

    while (!parentlessNodes.empty())
    {
        auto it = parentlessNodes.begin();
        uint8_t currentId = it->first.first & 0b111;
        uint64_t parentId = it->first.first >> 3;
        uint8_t depth = it->first.second - 1;

        std::pair<uint64_t, uint8_t> key = std::make_pair(parentId, depth);

        if (parentlessNodes.size() == 1 && it->first.second == 0)
        {
            root = it->second;
            break;
        }

        if (it->first.second == 0)
            continue;

        parentBuffer[key].push_back(it->second);
        parentlessNodes.erase(it);

        if (parentBuffer[key].size() == 8)
        {
            std::vector<OctreeNode *> children = parentBuffer[key];

            glm::vec3 lowerCorner = children[0]->getLowerBounds();
            glm::vec3 upperCorner = children[0]->getUpperBounds();

            for (size_t i = 1; i < children.size(); i++)
            {
                glm::vec3 childLower = children[i]->getLowerBounds();
                glm::vec3 childUpper = children[i]->getUpperBounds();

                lowerCorner.x = std::min(lowerCorner.x, childLower.x);
                lowerCorner.y = std::min(lowerCorner.y, childLower.y);
                lowerCorner.z = std::min(lowerCorner.z, childLower.z);

                upperCorner.x = std::max(upperCorner.x, childUpper.x);
                upperCorner.y = std::max(upperCorner.y, childUpper.y);
                upperCorner.z = std::max(upperCorner.z, childUpper.z);
            }

            OctreeNode *newNode = new OctreeNode(lowerCorner, upperCorner, depth, currentId);
            for (OctreeNode *child : children)
                newNode->addChild(child);

            parentlessNodes.emplace(key, newNode);
            parentBuffer.erase(key);
        }
    }

    if (root != nullptr)
    {
        Mesh *mesh = new Mesh(outFaces, outVertices, root);
        mesh->setName(meshName);
        std::cout << "[+] Mesh Loaded: " << mesh->getName() << std::endl;
        addNodeTo(getCurrentScene(), mesh);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "[>] Octree import took: " << duration.count() << " ms\n";
    std::cout << "[>] Octree import (excluding mesh load): " << duration.count() - meshImportTime << " ms\n";
}

void ENG_API Eng::Base::castRay(glm::vec3 startPos, glm::vec3 direction, float distance)
{
    Scene *scene = this->getCurrentScene();
    glm::vec3 endPos = startPos + glm::normalize(direction) * distance;

    // std::cout << "rayStart: (" << startPos.x << ", " << startPos.y << ", " << startPos.z << ")" << std::endl;
    // std::cout << "endPos: (" << endPos.x << ", " << endPos.y << ", " << endPos.z << ")" << std::endl;

    scene->setRay(startPos, endPos);
    if (getBenchmarkMode() == 1 || getBenchmarkMode() >= 3)
        checkIntersection(startPos, endPos); // optimized
    else if (getBenchmarkMode() == 2)
        checkIntersectionUnoptimized(startPos, endPos); // brute force
}

void ENG_API Eng::Base::castRay(int mouseX, int mouseY)
{
    Scene *scene = this->getCurrentScene();
    Camera *camera = scene->getCurrentCamera();
    Eng::CameraConfig cameraConfig = camera->getConfig();

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glm::mat4 projMatrix = camera->getProj();
    glm::mat4 viewMatrix = glm::inverse(camera->getTransform());

    GLdouble winX = (GLdouble)mouseX;
    GLdouble winY = (GLdouble)viewport[3] - (GLdouble)mouseY;

    double modelArray[16];
    double projArray[16];

    for (int i = 0; i < 16; i++)
    {
        modelArray[i] = viewMatrix[i / 4][i % 4];
        projArray[i] = projMatrix[i / 4][i % 4];
    }

    GLdouble nearX, nearY, nearZ;
    gluUnProject(winX, winY, 0.0f, modelArray, projArray, viewport, &nearX, &nearY, &nearZ);

    GLdouble farX, farY, farZ;
    gluUnProject(winX, winY, 1.0f, modelArray, projArray, viewport, &farX, &farY, &farZ);

    glm::vec3 rayStart = glm::vec3((GLfloat)nearX, (GLfloat)nearY, (GLfloat)nearZ);
    glm::vec3 rayEnd = glm::vec3((GLfloat)farX, (GLfloat)farY, (GLfloat)farZ);

    // std::cout << "rayStart: (" << rayStart.x << ", " << rayStart.y << ", " << rayStart.z << ")" << std::endl;
    // std::cout << "rayEnd: (" << rayEnd.x << ", " << rayEnd.y << ", " << rayEnd.z << ")" << std::endl;

    scene->setRay(rayStart, rayEnd);
    if (getBenchmarkMode() == 1 || getBenchmarkMode() >= 3)
        checkIntersection(rayStart, rayEnd); // optimized
    else if (getBenchmarkMode() == 2)
        checkIntersectionUnoptimized(rayStart, rayEnd); // brute force
}

bool rayIntersectsAABB(glm::vec3 orig, glm::vec3 dir, glm::vec3 mn, glm::vec3 mx)
{
    float tmin = 0.0f, tmax = 1.0f;
    for (int i = 0; i < 3; i++)
    {
        float d = dir[i];
        float o = orig[i];
        if (fabs(d) < 1e-8f)
        {
            if (o < mn[i] || o > mx[i])
                return false;
        }
        else
        {
            float t0 = (mn[i] - o) / d;
            float t1 = (mx[i] - o) / d;
            if (t0 > t1)
                std::swap(t0, t1);
            tmin = std::max(tmin, t0);
            tmax = std::min(tmax, t1);
            if (tmin > tmax)
                return false;
        }
    }
    return true;
}

void ENG_API Eng::Base::checkIntersection(glm::vec3 rayStart, glm::vec3 rayEnd)
{

    if (getCurrentScene()->getNrOfChildren() <= 2)
        return;

    Mesh *mesh = dynamic_cast<Mesh *>(getCurrentScene()->getChild(2));
    if (mesh == nullptr)
        return;

    mesh->clearFaceHit();
    mesh->clearNodesHit();

    OctreeNode *rootNode = mesh->getOctreeRoot();
    std::vector<OctreeNode *> stack = {rootNode};
    std::vector<OctreeNode *> savedNodes;
    float closestT = FLT_MAX;
    Face *closestFace = nullptr;

    // face check
    glm::vec3 orig(rayStart.x, rayStart.y, rayStart.z);
    glm::vec3 dir(
        rayEnd.x - rayStart.x,
        rayEnd.y - rayStart.y,
        rayEnd.z - rayStart.z);

    glm::vec3 intersectPos;

    while (!stack.empty())
    {
        OctreeNode *node = stack.back();
        stack.pop_back();

        if (!rayIntersectsAABB(orig, dir, node->getLowerBounds(), node->getUpperBounds()))
            continue;

        if (node->hasChildren())
        {
            for (OctreeNode *child : node->getChildren())
            {
                stack.push_back(child);
            }
        }
        else if (node->hasFaces())
        {
            savedNodes.push_back(node);

            for (const auto &f : node->getFaces())
            {
                const auto &v0 = f->_vertices[0];
                const auto &v1 = f->_vertices[1];
                const auto &v2 = f->_vertices[2];

                glm::vec3 vert0(v0->x, v0->y, v0->z);
                glm::vec3 vert1(v1->x, v1->y, v1->z);
                glm::vec3 vert2(v2->x, v2->y, v2->z);

                glm::vec3 edge1 = vert1 - vert0;
                glm::vec3 edge2 = vert2 - vert0;
                glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));
                glm::vec3 rayDir = glm::normalize(dir);

                // Face normal faces the ray
                if (glm::dot(faceNormal, rayDir) >= 0)
                    continue;

                if (intersectLineTriangle(orig, dir, vert0, vert1, vert2, intersectPos))
                {
                    // check if intersection is within ray segment (0 <= t <= 1)
                    float t = glm::length(intersectPos - orig) / glm::length(dir);
                    if (t >= 0 && t <= 1 && t < closestT)
                    {
                        closestT = t;
                        closestFace = f;
                    }
                }
            }
        }
    }

    if (closestFace)
    {
        mesh->setFaceHit(closestFace);
        mesh->setNodesHit(savedNodes);
    }
}

void ENG_API Eng::Base::checkIntersectionUnoptimized(glm::vec3 rayStart, glm::vec3 rayEnd)
{
    if (getCurrentScene()->getNrOfChildren() <= 2)
        return;
    Mesh *mesh = dynamic_cast<Mesh *>(getCurrentScene()->getChild(2));
    if (mesh == nullptr)
        return;
    mesh->clearFaceHit();
    mesh->clearNodesHit();

    glm::vec3 orig(rayStart.x, rayStart.y, rayStart.z);
    glm::vec3 dir(rayEnd.x - rayStart.x, rayEnd.y - rayStart.y, rayEnd.z - rayStart.z);
    glm::vec3 intersectPos;
    float closestT = FLT_MAX;
    Face *closestFace = nullptr;

    for (const auto &f : mesh->getFaces())
    {
        const auto &v0 = f->_vertices[0];
        const auto &v1 = f->_vertices[1];
        const auto &v2 = f->_vertices[2];
        glm::vec3 vert0(v0->x, v0->y, v0->z);
        glm::vec3 vert1(v1->x, v1->y, v1->z);
        glm::vec3 vert2(v2->x, v2->y, v2->z);
        glm::vec3 faceNormal = glm::normalize(glm::cross(vert1 - vert0, vert2 - vert0));
        if (glm::dot(faceNormal, glm::normalize(dir)) >= 0)
            continue;
        if (intersectLineTriangle(orig, dir, vert0, vert1, vert2, intersectPos))
        {
            float t = glm::length(intersectPos - orig) / glm::length(dir);
            if (t >= 0 && t <= 1 && t < closestT)
            {
                closestT = t;
                closestFace = f;
            }
        }
    }

    if (closestFace)
        mesh->setFaceHit(closestFace);
}