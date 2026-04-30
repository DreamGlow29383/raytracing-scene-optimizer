/**
 * @file scene.h
 * @brief Scene graph root and manager.
 */

#pragma once

#include "node.h"
#include "camera.h"
#include "frame_event.h"
#include "key_event.h"

#include <map>

 /**
  * @class Scene
  * @brief Represents the root node of a scenegraph.
  * * Manages the list of nodes, the active camera, lighting settings, and event dispatching.
  */
class Scene : public Node {
public:
	Scene();
	~Scene();

	/**
	 * @brief Sets the global ambient light color for the scene.
	 * @param ambient RGBA color vector.
	 */
	void setAmbient(glm::vec4 ambient);

	/**
	 * @brief Retrieves a node by its unique ID.
	 * @param id The unique ID of the node.
	 * @return Pointer to the Node, or nullptr if not found.
	 */
	Node* getNode(int id);

	/**
	 * @brief Updates the internal map of nodes.
	 * Usually called automatically when adding a node.
	 * @param node Pointer to the node to register.
	 */
	void updateMap(Node* node);

	/**
	 * @brief Gets the currently active camera used for rendering.
	 * @return Pointer to the Camera object.
	 */
	Camera* getCurrentCamera();

	/**
	 * @brief Sets the active camera for the scene.
	 * @param camera Pointer to the Camera object to use.
	 */
	void setCurrentCamera(Camera* camera);

	/**
	 * @brief Adds a line of text to the lines displayed at the bottom right.
	 * @param text Text to add to the displayed lines.
	 */
	void addText(std::string text);

	/**
	 * @brief Returns all text lines to be displayed
	 * @return Vector with all text lines.
	 */
	std::vector<std::string> getTextLines();

	/**
	 * @brief Binds a function to be called every frame.
	 * @param event Pointer to the FrameEvent object.
	 */
	void bindEvent(FrameEvent* event);

	/**
	 * @brief Binds a function to be called on a specific key press.
	 * @param key The character code of the key.
	 * @param event Pointer to the KeyEvent object.
	 */
	void bindEvent(char key, KeyEvent* event);

	/**
	 * @brief Triggers all bound frame events.
	 * @param deltaTime Time elapsed since the last frame.
	 */
	void fireFrameEvents(float deltaTime);

	/**
	 * @brief Triggers events bound to a key press.
	 * @param key The key that was pressed.
	 */
	void fireKeyPressedEvents(char key);

	/**
	 * @brief Triggers events bound to a key release.
	 * @param key The key that was released.
	 */
	void fireKeyReleasedEvents(char key);

	/**
	 * @brief Traverses the graph to build a linear list of renderable objects.
	 * Useful for optimization or state sorting.
	 */
	void computeRenderList();

	/**
	 * @brief Gets the computed list of renderable nodes.
	 * @return A vector of Node pointers.
	 */
	std::vector<Node*> getRenderList();

	/**
	 * @brief Renders the scene from the perspective of the current camera.
	 * @param cameraInverse The inverse of the camera's world transformation matrix.
	 */
	virtual void render(glm::mat4 cameraInverse) override;

	// Ray tracing simulation
	void setRay(glm::vec3 start, glm::vec3 end) {
		_rayStart = start;
		_rayEnd = end;
		_showRay = true;
	}

	void clearRay() {
		_showRay = false;
		_rayStart = glm::vec3(0.0f);
		_rayEnd = glm::vec3(0.0f);
	}

	bool hasRay() const { return _showRay; }

	glm::vec3 getRayStart() const {
		return _rayStart;
	}

	glm::vec3 getRayEnd() const {
		return _rayEnd;
	}
private:

	bool _showRay = false;
	glm::vec3 _rayStart;
	glm::vec3 _rayEnd;

	glm::vec4 _ambient;
	std::map<int, Node*> _nodes;
	Camera* _currentCamera;
	std::vector<Node*> _renderList;
	std::vector<FrameEvent*> _frameEvents;
	std::map<char, std::vector<KeyEvent*>> _keyEvents;
	std::vector<std::string> _textLines;

	void pass(Node* node);
};