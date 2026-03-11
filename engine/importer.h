/**
 * @file importer.h
 * @brief Utilities for importing 3D models.
 */

#pragma once

#include <vector>

#include "mesh.h"

 /**
  * @brief Imports a 3D model file (OBJ, FBX, etc.) into a list of Nodes.
  * @param filepath The path to the file on disk.
  * @return A vector of pointers to the created Nodes (Meshes).
  */
bool importFile(const std::string& filepath, std::vector<Vertex*>& outVertices, std::vector<Face*>& outFaces);