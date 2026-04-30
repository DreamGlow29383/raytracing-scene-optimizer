#define FREEIMAGE_LIB
#include "engine.h"
#include "callback.h"
#include "node.h"
#include "scene.h"
#include "light.h"
#include "camera.h"
#include "importer.h"
#include "frame_event.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>

#include <chrono>
#include <thread>
#include <iostream>   
#include <source_location>
#include <FreeImage.h>
#include <fstream>
#include <utility>

#include <GL/glew.h>
#include <GL/freeglut.h>

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

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
    
    glutInit(&argc, argv);

    int window_w = 800;
    int window_h = 600;

    int pos_x = (glutGet(GLUT_SCREEN_WIDTH) / 2) - (window_w / 2);
    int pos_y = (glutGet(GLUT_SCREEN_HEIGHT) / 2) - (window_h / 2);

    glutInitWindowSize(window_w, window_h);
    glutInitWindowPosition(pos_x, pos_y);

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    windowId = glutCreateWindow("Raytracing Octree Optimization");

    glewInit();

    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialCallback);
    glutMouseFunc(mouseCallback);

    glutSetOption(GLUT_MULTISAMPLE, 8);
    glEnable(GL_MULTISAMPLE); // enable anti-aliasing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

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

void ENG_API Eng::Base::castRaySurface(int mouseX, int mouseY) {
   Scene* scene = this->getCurrentScene();
   Camera* camera = scene->getCurrentCamera();
   Eng::CameraConfig cameraConfig = camera->getConfig();

   int viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   glm::mat4 projMatrix = camera->getProj();
   glm::mat4 viewMatrix = glm::inverse(camera->getTransform());

   GLdouble winX = (GLdouble)mouseX;
   GLdouble winY = (GLdouble)viewport[3] - (GLdouble)mouseY;

   double modelArray[16];
   double projArray[16];

   for (int i = 0; i < 16; i++) {
      modelArray[i] = viewMatrix[i / 4][i % 4];
      projArray[i] = projMatrix[i / 4][i % 4];
   }

   GLdouble nearX, nearY, nearZ;
   gluUnProject(winX, winY, 0.0f, modelArray, projArray, viewport, &nearX, &nearY, &nearZ);

   GLdouble farX, farY, farZ;
   gluUnProject(winX, winY, 1.0f, modelArray, projArray, viewport, &farX, &farY, &farZ);

   glm::vec3 rayStart = glm::vec3((GLfloat)nearX, (GLfloat)nearY, (GLfloat)nearZ);
   glm::vec3 rayEnd = glm::vec3((GLfloat)farX, (GLfloat)farY, (GLfloat)farZ);

   std::cout << "rayStart: (" << rayStart.x << ", " << rayStart.y << ", " << rayStart.z << ")" << std::endl;
   std::cout << "rayEnd: (" << rayEnd.x << ", " << rayEnd.y << ", " << rayEnd.z << ")" << std::endl;

   scene->setRay(rayStart, rayEnd);
   locateRaySurface();
}

void ENG_API Eng::Base::castRayThrough(int mouseX, int mouseY) {
   Scene* scene = this->getCurrentScene();
   Camera* camera = scene->getCurrentCamera();
   Eng::CameraConfig cameraConfig = camera->getConfig();

   int viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   glm::mat4 projMatrix = camera->getProj();
   glm::mat4 viewMatrix = glm::inverse(camera->getTransform());

   GLdouble winX = (GLdouble)mouseX;
   GLdouble winY = (GLdouble)viewport[3] - (GLdouble)mouseY;

   double modelArray[16];
   double projArray[16];

   for (int i = 0; i < 16; i++) {
      modelArray[i] = viewMatrix[i / 4][i % 4];
      projArray[i] = projMatrix[i / 4][i % 4];
   }

   GLdouble nearX, nearY, nearZ;
   gluUnProject(winX, winY, 0.0f, modelArray, projArray, viewport, &nearX, &nearY, &nearZ);

   GLdouble farX, farY, farZ;
   gluUnProject(winX, winY, 1.0f, modelArray, projArray, viewport, &farX, &farY, &farZ);

   glm::vec3 rayStart = glm::vec3((GLfloat)nearX, (GLfloat)nearY, (GLfloat)nearZ);
   glm::vec3 rayEnd = glm::vec3((GLfloat)farX, (GLfloat)farY, (GLfloat)farZ);

   std::cout << "rayStart: (" << rayStart.x << ", " << rayStart.y << ", " << rayStart.z << ")" << std::endl;
   std::cout << "rayEnd: (" << rayEnd.x << ", " << rayEnd.y << ", " << rayEnd.z << ")" << std::endl;

   scene->setRay(rayStart, rayEnd);
   locateRayThrough();
}

bool ENG_API Eng::Base::rayIntersectsNode(OctreeNode* node) {
   Scene* scene = this->getCurrentScene();

   glm::vec3 rayStart = scene->getRayStart();
   glm::vec3 rayEnd = scene->getRayEnd();   

   Vertex* rayStartVertex = new Vertex();
   rayStartVertex->x = rayStart.x;
   rayStartVertex->y = rayStart.y;
   rayStartVertex->z = rayStart.z;

   Vertex* rayEndVertex = new Vertex();
   rayEndVertex->x = rayEnd.x;
   rayEndVertex->y = rayEnd.y;
   rayEndVertex->z = rayEnd.z;

   Face* flatTriangle = new Face();
   flatTriangle->_vertices.push_back(rayStartVertex);
   flatTriangle->_vertices.push_back(rayEndVertex);
   flatTriangle->_vertices.push_back(rayEndVertex);

   return node->check(flatTriangle);
}

bool ENG_API Eng::Base::rayIntersectsFace(Face* face) {

   Scene* scene = this->getCurrentScene();

   glm::vec3 rayStart = scene->getRayStart();
   glm::vec3 rayEnd = scene->getRayEnd();

   Vertex* rayStartVertex = new Vertex();
   rayStartVertex->x = rayStart.x;
   rayStartVertex->y = rayStart.y;
   rayStartVertex->z = rayStart.z;

   Vertex* rayEndVertex = new Vertex();
   rayEndVertex->x = rayEnd.x;
   rayEndVertex->y = rayEnd.y;
   rayEndVertex->z = rayEnd.z;
   
      glm::vec3 orig(rayStart.x, rayStart.y, rayStart.z);
      glm::vec3 dir(
         rayEnd.x - rayStart.x,
         rayEnd.y - rayStart.y,
         rayEnd.z - rayStart.z
      );

      glm::vec3 intersectPos;

      Vertex* v0 = face->_vertices[0];
      Vertex* v1 = face->_vertices[1];
      Vertex* v2 = face->_vertices[2];

      glm::vec3 vert0(v0->x, v0->y, v0->z);
      glm::vec3 vert1(v1->x, v1->y, v1->z);
      glm::vec3 vert2(v2->x, v2->y, v2->z);

      if (intersectLineTriangle(orig, dir, vert0, vert1, vert2, intersectPos)) {
         // check if intersection is within ray segment (0 <= t <= 1)
         float t = glm::length(intersectPos - orig) / glm::length(dir);
         if (t >= 0 && t <= 1) {
            
            return true;
         }
      }
      return false;
   }

void ENG_API Eng::Base::renderRay() {
   if (_facesHit.empty() && _nodesHit.empty()) return;

   glDisable(GL_LIGHTING);
   glDisable(GL_TEXTURE_2D);

   // Render through mode (cubes)
   if (!_nodesHit.empty()) {
      for (OctreeNode* node : _nodesHit) {
         if (nodeColors.find(node) != nodeColors.end()) {
            renderNodeAsCube(node, nodeColors[node]);
         }
      }
   }
   // Render surface mode (faces)
   else if (!_facesHit.empty()) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(1.0f, 1.0f);

      for (size_t i = 0; i < _facesHit.size(); i++) {
         glm::vec3 col;
         if (i < colorType.size() && nodeColors.find(colorType[i]) != nodeColors.end()) {
            col = nodeColors[colorType[i]];
         }
         else {
            col = glm::vec3(1.0f, 0.0f, 0.0f); // Default red
         }

         glBegin(GL_TRIANGLES);
         glColor3f(col.r, col.g, col.b);
         glVertex3f(_facesHit[i]->_vertices[0]->x, _facesHit[i]->_vertices[0]->y, _facesHit[i]->_vertices[0]->z);
         glVertex3f(_facesHit[i]->_vertices[1]->x, _facesHit[i]->_vertices[1]->y, _facesHit[i]->_vertices[1]->z);
         glVertex3f(_facesHit[i]->_vertices[2]->x, _facesHit[i]->_vertices[2]->y, _facesHit[i]->_vertices[2]->z);
         glEnd();
      }

      glDisable(GL_POLYGON_OFFSET_FILL);
   }

   glEnable(GL_LIGHTING);
}

void ENG_API Eng::Base::locateRaySurface() {
   _facesHit.clear();
   _nodesHit.clear();
   colorType.clear();

   if (!getCurrentScene()->hasRay()) return;

   std::vector<OctreeNode*> stack = { rootNode };

   while (!stack.empty()) {
      OctreeNode* node = stack.back();
      stack.pop_back();

      if (!rayIntersectsNode(node)) continue;

      if (node->hasChildren()) {
         for (OctreeNode* child : node->getChildren()) {
            stack.push_back(child);
         }
      }
      else if (node->hasFaces()) {
         for (const auto& f : node->getFaces()) {
            if (rayIntersectsFace(f)) {
               _facesHit.push_back(f);
               _nodesHit.push_back(node);
               colorType.push_back(node);
               return; 
            }
         }
      }
   }
}

void ENG_API Eng::Base::locateRayThrough() {
   _facesHit.clear();
   _nodesHit.clear();
   colorType.clear();

   if (!getCurrentScene()->hasRay()) return;

   std::vector<OctreeNode*> stack = { rootNode };

   while (!stack.empty()) {
      OctreeNode* node = stack.back();
      stack.pop_back();

      if (!rayIntersectsNode(node)) continue;

      if (node->hasChildren()) {
         for (OctreeNode* child : node->getChildren()) {
            stack.push_back(child);
         }
      }
      else if (node->hasFaces()) {
         _nodesHit.push_back(node);

         for (const auto& f : node->getFaces()) {
            _facesHit.push_back(f);
            colorType.push_back(node);
         }
      }
   }

   std::cout << "Ray passes through " << _nodesHit.size() << " nodes" << std::endl;
}

void ENG_API Eng::Base::renderNodeAsCube(OctreeNode* node, glm::vec3 color) {
   glm::vec3 min = node->getLowerBounds();
   glm::vec3 max = node->getUpperBounds();

   glm::vec3 corners[8] = {
       glm::vec3(min.x, min.y, min.z), // 0
       glm::vec3(max.x, min.y, min.z), // 1
       glm::vec3(max.x, max.y, min.z), // 2
       glm::vec3(min.x, max.y, min.z), // 3
       glm::vec3(min.x, min.y, max.z), // 4
       glm::vec3(max.x, min.y, max.z), // 5
       glm::vec3(max.x, max.y, max.z), // 6
       glm::vec3(min.x, max.y, max.z)  // 7
   };

   int edges[12][2] = {
       {0,1}, {1,2}, {2,3}, {3,0}, // bottom face
       {4,5}, {5,6}, {6,7}, {7,4}, // top face
       {0,4}, {1,5}, {2,6}, {3,7}  // vertical edges
   };

   glColor3f(color.r, color.g, color.b);
   glBegin(GL_LINES);
   for (int i = 0; i < 12; i++) {
      glVertex3fv(&corners[edges[i][0]].x);
      glVertex3fv(&corners[edges[i][1]].x);
   }
   glEnd();

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glColor4f(color.r, color.g, color.b, 0.8f); // 20% transparency

   glBegin(GL_QUADS);
   // Bottom face
   glVertex3fv(&corners[0].x); glVertex3fv(&corners[1].x);
   glVertex3fv(&corners[2].x); glVertex3fv(&corners[3].x);
   // Top face
   glVertex3fv(&corners[4].x); glVertex3fv(&corners[5].x);
   glVertex3fv(&corners[6].x); glVertex3fv(&corners[7].x);
   // Front face
   glVertex3fv(&corners[0].x); glVertex3fv(&corners[1].x);
   glVertex3fv(&corners[5].x); glVertex3fv(&corners[4].x);
   // Back face
   glVertex3fv(&corners[3].x); glVertex3fv(&corners[2].x);
   glVertex3fv(&corners[6].x); glVertex3fv(&corners[7].x);
   // Left face
   glVertex3fv(&corners[0].x); glVertex3fv(&corners[3].x);
   glVertex3fv(&corners[7].x); glVertex3fv(&corners[4].x);
   // Right face
   glVertex3fv(&corners[1].x); glVertex3fv(&corners[2].x);
   glVertex3fv(&corners[6].x); glVertex3fv(&corners[5].x);
   glEnd();
   glDisable(GL_BLEND);
}