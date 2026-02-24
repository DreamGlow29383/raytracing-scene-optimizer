/**
 * @file frame_event.h
 * @brief Handles per-frame events associated with scene nodes.
 */
#pragma once

#include "engine.h"
#include <glm/glm.hpp>

class Node;

/**
 * @class FrameEvent
 * @brief Represents a frame event tied to a specific node and callback function.
 */
class FrameEvent {
public:
    /**
	 * @brief Constructs a FrameEvent with the given node and callback.
	 * @param node The node associated with the frame event.
	 * @param callback The callback function to be invoked each frame.
	 * @return A FrameEvent instance.
     */
    FrameEvent(Node* node, Eng::Base::FrameCallback callback);

    /**
	 * @brief Invokes the callback function with the given delta time.
	 * @param deltaTime The time elapsed since the last frame.
     */
    void invoke(float deltaTime);

    /**
	 * @brief Gets the associated node.
	 * @return Pointer to the associated Node.
     */
    Node* getNode() const;

    /**
	 * @brief Gets the associated callback function.
	 * @return The frame callback function.
     */
    Eng::Base::FrameCallback getCallback() const;

private:
    Node* _node;
    Eng::Base::FrameCallback _callback;
};