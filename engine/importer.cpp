#define _CRT_SECURE_NO_WARNINGS
#include "importer.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

// --- Forward Declarations ---
Node* createNodeHierarchy(aiNode* aiNode, const aiScene* scene);

// --- Helper method that convert matrix from Assimp -> GLM ---
glm::mat4 convertMatrix(const aiMatrix4x4& from) 
{
    glm::mat4 converted;
    // Assimp is Row-Major, GLM is Column-Major. 
    // We copy by transposing the access for the allignment of the cells correctly.
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            converted[i][j] = from[j][i];
        }
    }

    return converted;
}

Mesh* createMeshFromAssimp(const aiMesh* ai_mesh, const aiScene* scene)
{
    Mesh* mesh = new Mesh();
    mesh->setName(ai_mesh->mName.C_Str());

    std::vector<Vertex> vertices;
    vertices.reserve(ai_mesh->mNumVertices);

    for (unsigned int j = 0; j < ai_mesh->mNumVertices; j++)
    {
        Vertex vertex;
        vertex.x = ai_mesh->mVertices[j].x;
        vertex.y = ai_mesh->mVertices[j].y;
        vertex.z = ai_mesh->mVertices[j].z;

        if (ai_mesh->HasNormals()) {
            vertex.nx = ai_mesh->mNormals[j].x;
            vertex.ny = ai_mesh->mNormals[j].y;
            vertex.nz = ai_mesh->mNormals[j].z;
        }
        else
        {
            vertex.nx = 0.0f; vertex.ny = 1.0f; vertex.nz = 0.0f;
        }

        if (ai_mesh->HasTextureCoords(0)) {
            vertex.u = ai_mesh->mTextureCoords[0][j].x;
            vertex.v = ai_mesh->mTextureCoords[0][j].y;
        }
        else
        {
            vertex.u = 0.0f; vertex.v = 0.0f;
        }

        vertices.push_back(vertex);
    }

    std::vector<unsigned int> indices;
    for (unsigned int j = 0; j < ai_mesh->mNumFaces; j++)
    {
        aiFace face = ai_mesh->mFaces[j];
        for (unsigned int k = 0; k < face.mNumIndices; k++)
            indices.push_back(face.mIndices[k]);
    }

    mesh->setVertices(vertices, indices);
    return mesh;
}

Node* createNodeHierarchy(aiNode* aiNode, const aiScene* scene)
{
    Node* node = new Node();
    node->setName(aiNode->mName.C_Str());

    node->setTransform(convertMatrix(aiNode->mTransformation));

    for (unsigned int i = 0; i < aiNode->mNumMeshes; i++)
    {
        unsigned int meshIndex = aiNode->mMeshes[i];
        Mesh* meshNode = createMeshFromAssimp(scene->mMeshes[meshIndex],scene);
        node->addChild(meshNode);
    }

    for (unsigned int i = 0; i < aiNode->mNumChildren; i++)
    {
        Node* childNode = createNodeHierarchy(aiNode->mChildren[i], scene);
        if (childNode)
        {
            node->addChild(childNode);
        }
    }

    return node;
}

std::vector<Node*> importFile(const std::string& filepath)
{
    std::vector<Node*> rootNodes;
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filepath,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices
    );

    if (!scene || !scene->mRootNode)
    {
        std::cout << "Failed to load file: " << importer.GetErrorString() << std::endl;
        return rootNodes;
    }

    glm::mat4 rootTransform = convertMatrix(scene->mRootNode->mTransformation);

    //If there are meshes attached directly to the root, the cycle of the children 
    //will be skipped. We create them manually here:
    if (scene->mRootNode->mNumMeshes > 0)
    {
        Node* rootGeometryNode = new Node();
        rootGeometryNode->setName(scene->mRootNode->mName.C_Str());

        //Applying the global transformation of the root
        rootGeometryNode->setTransform(rootTransform);

        for (unsigned int i = 0; i < scene->mRootNode->mNumMeshes; i++)
        {
            unsigned int meshIndex = scene->mRootNode->mMeshes[i];
 
            Mesh* mesh = createMeshFromAssimp(scene->mMeshes[meshIndex], scene);
            rootGeometryNode->addChild(mesh);
        }

        std::cout << "[Importer] Found meshes in the root node!" << std::endl;
        rootNodes.push_back(rootGeometryNode);
    }

    for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; i++)
    {
        Node* childNode = createNodeHierarchy(scene->mRootNode->mChildren[i], scene);

        if (childNode)
        {
            glm::mat4 finalTransform = rootTransform * childNode->getTransform();
            childNode->setTransform(finalTransform);

            rootNodes.push_back(childNode);
        }
    }

    return rootNodes;
}