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
#include <bitset>

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
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);

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

void ENG_API Eng::Base::removeNode(int id) {
    Scene* scene = getCurrentScene();
    for (int i = 0; i < scene->getNrOfChildren(); i++) {
        Node* current = scene->getChild(i);
        if (current->getId() == id) {
            scene->removeChild(i);
            break;
        }
    }
}

int ENG_API Eng::Base::addNodeFromFile(int parent, const std::string& filepath) {
    std::vector<Vertex*> outVertices;
    std::vector<Face*> outFaces;

    bool success = importFile(filepath, outVertices, outFaces);
    std::string meshName = filepath.substr(filepath.find_last_of("\\") + 1);

    if (success) {
        Mesh* mesh = new Mesh(outFaces, outVertices);
        mesh->setName(meshName);
        std::cout << "[+] Mesh Loaded: " << mesh->getName() << std::endl;
        std::cout << "[?] Faces: " << outFaces.size() << std::endl;
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

void ENG_API Eng::Base::setColoringMode(int mode) {
    coloring_mode = mode;

    Scene* scene = this->getCurrentScene();
    Mesh* mesh = dynamic_cast<Mesh*>(scene->getChild(2));

    mesh->updateColorVBO(coloring_mode);
}

std::vector<std::pair<uint64_t, OctreeNode*>> exportedNodes;
void traverseOctree(uint64_t completeId, OctreeNode* node) {
    if (!node->hasChildren())
        exportedNodes.push_back({ completeId, node });

    if (node->hasChildren()) {
        for (OctreeNode* child : node->getChildren()) {
            uint64_t newId = (completeId << 3) | child->getId();
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

    Scene* scene = this->getCurrentScene();
    Mesh* mesh = dynamic_cast<Mesh*>(scene->getChild(2));
    std::vector<Face*> mesh_faces = mesh->getFaces();

    std::string meshName = mesh ? mesh->getName() : "Unknown";
    uint8_t nameLength = static_cast<uint8_t>(meshName.length());

    file.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
    file.write(meshName.c_str(), meshName.length());

    const uint8_t padding = (4 - (meshName.length() % 4)) % 4;
    for (uint8_t i = 0; i < padding; ++i) {
        const char zero = 0;
        file.write(&zero, sizeof(zero));
    }

    // Write Octree Data
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

            //std::cout << "Node ID: " << std::bitset<64>(pair.first) << std::endl;
            //std::cout << "Depth: " << (int)node_depth << std::endl;
            //std::cout << "Faces: " << faces.size() << std::endl;

            glm::vec3 upper = node->getUpperBounds();
            glm::vec3 lower = node->getLowerBounds();
            
            file.write(reinterpret_cast<const char*>(&node_depth), sizeof(node_depth));
            file.write(reinterpret_cast<const char*>(&id), sizeof(id));
            file.write(reinterpret_cast<const char*>(&n_faces), sizeof(n_faces));

            file.write(reinterpret_cast<const char*>(&upper.x), sizeof(upper.x));
            file.write(reinterpret_cast<const char*>(&upper.y), sizeof(upper.y));
            file.write(reinterpret_cast<const char*>(&upper.z), sizeof(upper.z));

            file.write(reinterpret_cast<const char*>(&lower.x), sizeof(lower.x));
            file.write(reinterpret_cast<const char*>(&lower.y), sizeof(lower.y));
            file.write(reinterpret_cast<const char*>(&lower.z), sizeof(lower.z));

            glm:

            for (int i = 0; i < faces.size(); i++) {
                auto it = std::find(mesh_faces.begin(), mesh_faces.end(), faces[i]);
                uint32_t globalIndex = std::distance(mesh_faces.begin(), it);
                uint32_t swp_index = be32toh(globalIndex);
                file.write(reinterpret_cast<const char*>(&swp_index), sizeof(swp_index));
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

struct NodeData {
    uint64_t id;
    uint8_t depth;
    glm::vec3 upperCorner;
    glm::vec3 lowerCorner;
    std::vector<uint32_t> faceIndices;
};

void ENG_API Eng::Base::importOctree(const std::string& infilepath) {
    std::ifstream file(infilepath, std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + infilepath);
    }

    uint32_t magic;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    magic = be32toh(magic);

    if (magic != 0x4F435452) { // "OCTR"
        throw std::runtime_error("Invalid file format: Wrong magic number");
    }

    uint8_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));

    if (version != 1) {
        throw std::runtime_error("Unsupported file version: " + std::to_string(version));
    }

    uint8_t max_depth;
    file.read(reinterpret_cast<char*>(&max_depth), sizeof(max_depth));

    std::cout << "Octree file info:" << std::endl;
    std::cout << "  Version: " << (int)version << std::endl;
    std::cout << "  Max Depth: " << (int)max_depth << std::endl;

    uint8_t nameLength;
    file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));

    std::string meshName(nameLength, '\0');
    file.read(&meshName[0], nameLength);

    uint8_t padding = (4 - (nameLength % 4)) % 4;
    file.seekg(padding, std::ios::cur);

    std::cout << "  Mesh Name: " << meshName << std::endl;

    std::vector<NodeData*> nodes;

    while (file.peek() != EOF) {
        uint8_t node_depth;
        uint64_t node_id;
        uint32_t n_faces;

        float upperX, upperY, upperZ;
        float lowerX, lowerY, lowerZ;

        file.read(reinterpret_cast<char*>(&node_depth), sizeof(node_depth));
        file.read(reinterpret_cast<char*>(&node_id), sizeof(node_id));
        file.read(reinterpret_cast<char*>(&n_faces), sizeof(n_faces));

        file.read(reinterpret_cast<char*>(&upperX), sizeof(upperX));
        file.read(reinterpret_cast<char*>(&upperY), sizeof(upperY));
        file.read(reinterpret_cast<char*>(&upperZ), sizeof(upperZ));
        file.read(reinterpret_cast<char*>(&lowerX), sizeof(lowerX));
        file.read(reinterpret_cast<char*>(&lowerY), sizeof(lowerY));
        file.read(reinterpret_cast<char*>(&lowerZ), sizeof(lowerZ));

        node_id = be64toh(node_id);
        n_faces = be32toh(n_faces);

        std::vector<uint32_t> faceIndices;

        for (uint32_t f = 0; f < n_faces; ++f) {
            uint32_t index;
            file.read(reinterpret_cast<char*>(&index), sizeof(index));
            faceIndices.push_back(be32toh(index));
        }

        NodeData* data = new NodeData(node_id, node_depth, glm::vec3(upperX, upperY, upperZ), glm::vec3(lowerX, lowerY, lowerZ), faceIndices);
        nodes.push_back(data);

        //std::cout << "Node ID: " << std::bitset<64>(node_id) << std::endl;
        //std::cout << "Depth: " << (int)node_depth << std::endl;
        //std::cout << "Faces: " << n_faces << std::endl;
    }

    std::cout << "Total nodes loaded: " << nodes.size() << std::endl;

    // Now rebuild the mesh and octree from the loaded data

    size_t lastSlash = infilepath.find_last_of("\\");
    std::string directory = (lastSlash != std::string::npos)
        ? infilepath.substr(0, lastSlash + 1)
        : "";

    std::string meshpath = directory + meshName;

    std::vector<Vertex*> outVertices;
    std::vector<Face*> outFaces;
    bool success = importFile(meshpath, outVertices, outFaces);

    // Key: Node ID, Depth -> Value: The Octree Node
    std::map<std::pair<uint64_t, uint8_t>, OctreeNode*> parentlessNodes;
    std::map<std::pair<uint64_t, uint8_t>, std::vector<OctreeNode*>> parentBuffer;

    // Populate processedNodes with all leaves of the octree
    for (NodeData* data : nodes) {
        uint8_t currentId = data->id & 0b111;
        
        OctreeNode* newNode = new OctreeNode(data->lowerCorner, data->upperCorner, data->depth, currentId);
        
        std::vector<Face*> nodeFaces;
        for (uint32_t face_id : data->faceIndices) {
            nodeFaces.push_back(outFaces[face_id]);
        }

        newNode->setFaces(nodeFaces);

        auto key = std::make_pair(data->id, data->depth);
        parentlessNodes.emplace(key, newNode);
    }

    // Reconstruct the rest of the octree
    OctreeNode* root = nullptr;

    while (!parentlessNodes.empty()) {
        auto it = parentlessNodes.begin();
        uint8_t currentId = it->first.first & 0b111;
        uint64_t parentId = it->first.first >> 3;
        uint8_t depth = it->first.second - 1;

        std::pair<uint64_t, uint8_t> key = std::make_pair(parentId, depth);

        if (parentlessNodes.size() == 1 && it->first.second == 0) {
            root = it->second;
            break;
        }

        if (it->first.second == 0)
            continue;

        parentBuffer[key].push_back(it->second);
        parentlessNodes.erase(it);
        if (parentBuffer[key].size() == 8) {
            std::vector<OctreeNode*> children = parentBuffer[key];

            glm::vec3 lowerCorner = children[0]->getLowerBounds();
            glm::vec3 upperCorner = children[0]->getUpperBounds();

            for (size_t i = 1; i < children.size(); i++) {
                glm::vec3 childLower = children[i]->getLowerBounds();
                glm::vec3 childUpper = children[i]->getUpperBounds();

                lowerCorner.x = std::min(lowerCorner.x, childLower.x);
                lowerCorner.y = std::min(lowerCorner.y, childLower.y);
                lowerCorner.z = std::min(lowerCorner.z, childLower.z);

                upperCorner.x = std::max(upperCorner.x, childUpper.x);
                upperCorner.y = std::max(upperCorner.y, childUpper.y);
                upperCorner.z = std::max(upperCorner.z, childUpper.z);
            }


            OctreeNode* newNode = new OctreeNode(lowerCorner, upperCorner, depth, currentId);
            for (OctreeNode* child : children)
                newNode->addChild(child);

            parentlessNodes.emplace(key, newNode);
            parentBuffer.erase(key);
        }
    }

    if (root != nullptr) {
        Mesh* mesh = new Mesh(outFaces, outVertices, root);
        mesh->setName(meshName);
        std::cout << "[+] Mesh Loaded: " << mesh->getName() << std::endl;
        addNodeTo(getCurrentScene(), mesh);
    }

    file.close();
}