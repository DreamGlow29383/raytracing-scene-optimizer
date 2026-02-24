#include "object.h"

int Object::_idcounter = 0;

Object::Object() {
    this->_id = _idcounter;
    _idcounter++;
}

Object::~Object() {

}

std::uint32_t Object::getId() const
{
    return _id;
}

void Object::setName(std::string name)
{
    _name = name;
}

std::string Object::getName() const
{
    return _name;
}
