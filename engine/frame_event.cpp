#include "frame_event.h"
#include "node.h"

FrameEvent::FrameEvent(Node* node, Eng::Base::FrameCallback callback)
    : _node(node), _callback(callback) {
}

void FrameEvent::invoke(float deltaTime) {
    if (_callback && _node) {
        _callback(_node->getId(), deltaTime, _node->getTransform());
    }
}

Node* FrameEvent::getNode() const {
    return _node;
}

Eng::Base::FrameCallback FrameEvent::getCallback() const {
    return _callback;
}