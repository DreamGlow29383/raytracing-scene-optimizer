/**
 * @file object.h
 * @brief Base class for all identified entities in the engine.
 */

#pragma once

#include "engine.h"

#include <cstdint>
#include <string>

 /**
 * @class Object
 * @brief Abstract base class representing a generic entity with an ID and a name.
 */
class ENG_API Object
{
public:
	Object();
	~Object();

	/**
	 * @brief Gets the unique identifier of the object.
	 * @return The unique ID generated at creation.
	 */
	std::uint32_t getId() const;

	/**
	 * @brief Sets a human-readable name for the object.
	 * @param name The name string.
	 */
	void setName(std::string name);

	/**
	 * @brief Gets the object's name.
	 * @return The name string.
	 */
	std::string getName() const;

	/**
	 * @brief Pure virtual render function.
	 * Must be implemented by derived classes (even if they don't draw anything visually).
	 * @param cameraInverse The current view matrix.
	 */
	virtual void render(glm::mat4 cameraInverse) = 0;

private:
	static int _idcounter;
	std::uint32_t _id;
	std::string _name;
};
