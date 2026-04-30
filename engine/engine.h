/**
 * @file		engine.h
 * @brief	Graphics engine main include file
 *
 * @author	Ruben Barros (C) SUPSI [ruben.barros@student.supsi.ch]
 * @author	Sebastiano Piubellini (C) SUPSI [sebastiano.piubellin@student.supsi.ch]
 */

#pragma once

#include <memory> 
#include <unordered_map>
#include <string>
#include <functional>
#include <glm/glm.hpp>
#include "face.h"

class Node;
class Scene;
class OctreeNode;

#ifdef _DEBUG
    #define LIB_NAME        "My Graphics Engine v0.1a (debug)"
#else
    #define LIB_NAME        "My Graphics Engine v0.1a"
#endif
    #define LIB_VERSION     10 // (divide by 10)

#ifdef _WINDOWS
    #ifdef ENGINE_EXPORTS
        #define ENG_API __declspec(dllexport)
    #else
        #define ENG_API __declspec(dllimport)
    #endif     

    #pragma warning(disable : 4251) 
#else
    #define ENG_API
#endif

namespace Eng {

    /**
	 * @brief Light types enumeration
     */
    enum LightType {
        DIRECTIONAL,
        POINT,
        SPOT
    };

    /**
	 * @brief Camera types enumeration
     */
    enum CameraType {
        PERSPECTIVE,
        ORTHOGRAPHIC
    };

	/**
	 * @brief Shadow plane types enumeration
     */
    enum ShadowPlaneType {
        ENDLESS,
        CIRCLE
    };

    /**
    * @brief Configuration structure for Lights.
    */
    struct LightConfig {
        LightType type = LightType::POINT;
        glm::vec3 attenuation = glm::vec3(1.0f, 0.045f, 0.0075f);
        float cutoff = 30.0f;
        glm::vec3 ambient = glm::vec3(0.1f, 0.1f, 0.1f);
        glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
        float intensity = 2.0f;
    };

	/**
	 * @brief Configuration structure for Cameras.
     */
    struct CameraConfig {
        CameraType type = CameraType::PERSPECTIVE;
        float fov = glm::radians(45.0f);
        float left = 10.0f;
        float right = 10.0f;
        float bottom = 10.0f;
        float top = 10.0f;
        float nearPlane = 1.0f;
        float farPlane = 100.0f;
    };

    /**
	 * @brief Configuration structure for Shadow Planes.
     */
    struct ShadowPlaneConfig {
        ShadowPlaneType type = ShadowPlaneType::ENDLESS;
        float radius = 5.0f;
        glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
        float offset = 0.05f;
    };

/**
 * @brief Base engine main class. This class is a singleton.
 * * Manages the lifecycle of the application, scenes, nodes, and main loop.
 */
class ENG_API Base final
{
public:
    Base(Base const &) = delete;
    ~Base();

    /**
	 * @brief Deleted assignment operator to prevent copying.
	 * @param other The other Base instance.
     */
    void operator=(Base const &) = delete;

    /**
     * @brief Gets the singleton instance of the engine.
     * @return Reference to the Base instance.
     */
    static Base &getInstance();

    /**
	 * @brief Frame event callback type definition.
	 * @param int Node ID.
	 * @param float Delta time since last frame.
	 * @param glm::mat4 Node transformation matrix.
	 * @return void.
     */
    using FrameCallback = std::function<void(int, float, glm::mat4)>;

    /**
	 * @brief Key event callback type definition.
	 * @param int Node ID.
	 * @param bool True if key is pressed, false if released.
	 * @param glm::mat4 Node transformation matrix.
	 * @return void.
     */
    using KeyCallback = std::function<void(int, bool, glm::mat4)>;

    /**
     * @brief Initializes the engine and the underlying graphics library.
     * @param argc Command line argument count.
     * @param argv Command line arguments.
     * @return True if initialization was successful, false otherwise.
     */
    bool init(int argc, char* argv[]);

    /**
     * @brief Starts the main loop of the engine.
     * @return True if the loop finished successfully.
     */
    bool run();

    /**
     * @brief Frees resources and shuts down the engine.
     * @return True if shutdown was successful.
     */
    bool free();   

    /**
     * @brief Creates a new empty scene.
     * @return The unique ID of the created scene.
     */
    int createScene();

    /**
     * @brief Sets the active scene to be rendered.
     * @param sceneId The ID of the scene to make active.
     */
    void setCurrentScene(int sceneId);

    /**
     * @brief Sets the global ambient light color for the current scene.
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     * @param a Alpha component.
     */
    void setSceneAmbient(float r, float g, float b, float a);

    /**
     * @brief Gets the pointer to the currently active scene object.
     * @return Pointer to the current Scene.
     */
    Scene* getCurrentScene();

    /**
     * @brief Sets the active camera for the current scene using a Node ID.
     * @param id The ID of the node containing the camera.
     */
    void setSceneCamera(int id);

    /**
	 * @brief Adds a light node as a child of the specified parent node.
	 * @param parent The ID of the parent node.
	 * @return The ID of the newly created light node.
     */
    int addNodeLight(int parent, LightConfig config);

    /**
	 * @brief Adds a light node to the scene.
	 * @param config The configuration for the light node.
	 * @return The ID of the newly created light node.
     */
    int addNodeLight(LightConfig config);

    /**
	 * @brief Sets the light color of a node.
	 * @param id The ID of the node.
	 * @param color The RGB color vector to set.
     */
    void setNodeLightColor(int id, glm::vec3 color);

    /**
	 * @brief Adds a camera node as a child of the specified parent node.
	 * @param parent The ID of the parent node.
	 * @param config The configuration for the camera node.
	 * @return The ID of the newly created camera node.
     */
    int addNodeCamera(int parent, CameraConfig config);

    /**
	 * @brief Adds a camera node to the scene.
	 * @param config The configuration for the camera node.
	 * @return The ID of the newly created camera node.
     */
    int addNodeCamera(CameraConfig config);

    void removeNode(int id);

    /**
     * @brief Loads a 3D model from file and adds it to the scene.
     * @param parent The ID of the parent node.
     * @param filepath Path to the 3D model file.
     * @return The ID of the newly created root node for the model.
     */
    int addNodeFromFile(int parent, const std::string& filepath);

	/**
	 * @brief Loads a 3D model from file and adds it to the scene.
	 * @param filepath Path to the 3D model file.
	 * @return The ID of the newly created root node for the model.
     */
    int addNodeFromFile(const std::string& filepath);

    /**
     * @brief Add text to be displayed on screen.
     * @param text The text to add.
     */
    void addSceneText(std::string text);

    /**
	 * @brief Sets the transformation matrix for a node.
	 * @param id The ID of the node.
	 * @param transform The transformation matrix to set.
     */
    void setNodeTransform(int id, glm::mat4 transform);

    /**
	 * @brief Gets the transformation matrix of a node.
	 * @param id The ID of the node.
     */
    glm::mat4 getNodeTransform(int id);

    /**
	 * @brief Binds a frame event callback to a node's camera.
	 * @param nodeId The ID of the node.
	 * @param func The callback function to bind.
     */
    void bindSceneEvent(int nodeId, FrameCallback func);

    /**
	 * @brief Binds a key event callback to a node's camera.
	 * @param key The key character to bind the event to.
	 * @param nodeId The ID of the node.
	 * @param func The callback function to bind.
     */
    void bindSceneEvent(char key, int nodeId, KeyCallback func);

    /**
	 * @brief Gets the window ID used by the underlying graphics library.
	 * @return The window ID.
     */
    int getWindowId();

    void exportOctree(const std::string& outfilepath);
    void importOctree(const std::string& infilepath);

    void setShowNodeBoundaries(bool show) { show_node_boundaries = show; }
    bool getShowNodeBoundaries() const { return show_node_boundaries; }

    void setColoringMode(int mode);
    int getColoringMode() const { return coloring_mode; }

    void setBenchmarkMode(int mode) { benchmark_mode = mode; }
    int getBenchmarkMode() const { return benchmark_mode; }

    void setShowColoringSubmenu(bool show) { show_coloring_submenu = show; }
    bool getShowColoringSubmenu() const { return show_coloring_submenu; }

    void castRaySurface(int x, int y);

    void castRayThrough(int x, int y);

    bool rayIntersectsNode(OctreeNode* node);

    bool rayIntersectsFace(Face* face);

    void setRayIntersectsFaceFlag() {
       rayIntersectsFaceFlag = true;
    }
    void resetRayIntersectsFaceFlag() {
       rayIntersectsFaceFlag = false;
    }
    bool getRayIntersectsFaceFlag() {
       return rayIntersectsFaceFlag;
    }

    void setRootNode(OctreeNode* rootNode) {
       this->rootNode = rootNode;
    }

    void addNodeColor(OctreeNode* node, glm::vec3 color) {
       this->nodeColors[node] = color;
    }

    void renderRay();

private:
    struct Reserved;
    std::unique_ptr<Reserved> reserved;

    std::unordered_map<int, Scene*> scenes;
    int nextSceneId = 0;
    int currentSceneId = -1;
    int windowId;
    bool rayIntersectsFaceFlag;

    OctreeNode* rootNode;
    std::vector<OctreeNode*> colorType;
    std::vector<Face*> _facesHit;
    std::vector<OctreeNode*> _nodesHit;
    std::unordered_map<OctreeNode*, glm::vec3> nodeColors;

    bool show_node_boundaries = false;
    bool show_coloring_submenu = false;
    int coloring_mode = 0;
    int benchmark_mode = 0;

    void addNode(Node* node);
    void addNodeTo(Node* parent, Node* node);
    void locateRayThrough();
    void locateRaySurface();
    void renderNodeAsCube(OctreeNode* node, glm::vec3 color);

    Base();
};

};
