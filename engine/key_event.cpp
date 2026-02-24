#include "key_event.h"
#include "node.h"

KeyEvent::KeyEvent(Node* node, Eng::Base::KeyCallback callback)
    : _node(node), _callback(callback) {
}

void KeyEvent::invoke(bool keyDown) {
    if (_callback && _node) {
        _callback(_node->getId(), keyDown, _node->getTransform());
    }
}

Node* KeyEvent::getNode() const {
    return _node;
}

Eng::Base::KeyCallback KeyEvent::getCallback() const {
    return _callback;
}