/**
 * @file node.h
 * @brief Scene Graph Node implementation.
 */

#pragma once

#include "object.h"

#include <cstdint>
#include <string>
#include <vector>
    
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

 /**
  * @class Node
  * @brief Base class for all objects in the scene graph.
  * * Handles parent-child relationships and hierarchical transformations.
  */
class ENG_API Node : public Object
{
public:
	Node();
	~Node();

	/**
	 * @brief Adds a child node to this node.
	 * @param child Pointer to the child node.
	 */
	virtual void addChild(Node* child);

	/**
	 * @brief Removes a child node by index.
	 * @param idx Index of the child in the list.
	 * @return Pointer to the removed node (ownership is transferred back).
	 */
	virtual Node* removeChild(std::uint32_t idx);

	/**
	 * @brief Gets a child node by index.
	 * @param idx Index of the child in the list.
	 * @return Pointer to the child node.
	 */
	Node* getChild(std::uint32_t idx) const;

	/**
	 * @brief Gets the number of child nodes.
	 * @return Number of children.
	 */
	std::uint32_t getNrOfChildren(void) const;

	/**
	 * @brief Gets the parent node.
	 * @return Pointer to the parent node.
	 */
	Node* getParent() const;

	/**
	 * @brief Sets the parent node.
	 * @param parent Pointer to the new parent node.
	 */
	void setParent(Node* parent);

	/**
	 * @brief Renders the node and its children.
	 * @param cameraInverse The inverse of the camera's world transformation matrix.
	 */
	virtual void render(glm::mat4 cameraInverse) override;

	/**
	 * @brief Sets the local transformation matrix of the node.
	 * @param transform The transformation matrix to set.
	 */
	void setTransform(glm::mat4 transform);

	/**
	 * @brief Gets the local transformation matrix of the node.
	 * @return The local transformation matrix.
	 */
	glm::mat4 getTransform(void) const;

	/**
	 * @brief Computes and returns the world transformation matrix of the node.
	 * @return The world transformation matrix.
	 */
	glm::mat4 getWC(void) const;

private:
	glm::mat4 _transform;
	Node* _parent;
	std::vector<Node*> _children;
};
