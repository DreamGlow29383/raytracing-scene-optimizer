#include "node.h"

#include <GL/freeglut.h>
#include <iostream>

ENG_API Node::Node()
{
	_transform = glm::mat4(1.0f);
	_parent = nullptr;
}

ENG_API Node::~Node()
{
	
}

void ENG_API  Node::addChild(Node* child)
{
	_children.push_back(child);
	child->setParent(this);
}

Node ENG_API *Node::removeChild(std::uint32_t idx)
{
	Node* node = getChild(idx);
	_children.erase(_children.begin() + idx);
	node->setParent(nullptr);
	return node;
}

Node ENG_API *Node::getChild(std::uint32_t idx) const
{
	return _children.at(idx);
}

std::uint32_t ENG_API Node::getNrOfChildren(void) const
{
	return _children.size();
}

Node ENG_API *Node::getParent() const
{
	return _parent;
}

void ENG_API Node::setParent(Node* parent)
{
	_parent = parent;
}

void ENG_API Node::setTransform(glm::mat4 transform)
{
	_transform = transform;
}

glm::mat4 ENG_API Node::getTransform(void) const
{
	return _transform;
}

glm::mat4 ENG_API Node::getWC(void) const
{
	glm::mat4 wc = glm::mat4(1.0f);
	if (getParent() != NULL)
		wc = wc * getParent()->getWC();
	wc = wc * _transform;
	return wc;
}

void ENG_API Node::render(glm::mat4 cameraInverse)
{

}
