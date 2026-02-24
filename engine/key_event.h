/**
 * @file key_event.h
 * @brief Handles keyboard input events associated with scene nodes.
 */

#pragma once

#include "engine.h"
#include <glm/glm.hpp>

class Node;

/**
 * @class KeyEvent
 * @brief Encapsulates a callback to be executed on key press/release.
 */
class KeyEvent {
public:
    /**
     * @brief Constructs a key event.
     * @param node The node associated with this event.
     * @param callback The function to call when the key event triggers.
	 * @return A KeyEvent instance.
     */
    KeyEvent(Node* node, Eng::Base::KeyCallback callback);

    /**
     * @brief Invokes the callback function with the given key state.
     * @param keyDown True if the key is pressed, false if released.
	 */
    void invoke(bool keyDown);

    /**
     * @brief Gets the associated node.
     * @return Pointer to the associated Node.
	 */
    Node* getNode() const;
    
    /**
	 * @brief Gets the associated callback function.
	 * @return The key callback function.
     */
    Eng::Base::KeyCallback getCallback() const;

private:
    Node* _node;
    Eng::Base::KeyCallback _callback;
};