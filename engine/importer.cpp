#define _CRT_SECURE_NO_WARNINGS
#include "importer.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "face.h"

#include <iostream>
#include <vector>

bool importFile(const std::string& filepath, std::vector<Vertex*>& outVertices, std::vector<Face*> &outFaces)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filepath,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices
    );

    if (!scene || !scene->mRootNode)
    {
        std::cout << "Failed to load file: " << importer.GetErrorString() << std::endl;
        return false;
    }

    for (unsigned int i = 0; i == 0 /*< scene->mNumMeshes*/; i++)
    {
        const aiMesh* ai_mesh = scene->mMeshes[i];
        //unsigned int indexOffset = static_cast<unsigned int>(outVertices.size());

        for (unsigned int j = 0; j < ai_mesh->mNumVertices; j++)
        {
            Vertex* vertex = new Vertex;
            vertex->x = ai_mesh->mVertices[j].x;
            vertex->y = ai_mesh->mVertices[j].y;
            vertex->z = ai_mesh->mVertices[j].z;

            if (ai_mesh->HasNormals()) {
                vertex->nx = ai_mesh->mNormals[j].x;
                vertex->ny = ai_mesh->mNormals[j].y;
                vertex->nz = ai_mesh->mNormals[j].z;
            }
            else {
                vertex->nx = 0.0f; vertex->ny = 1.0f; vertex->nz = 0.0f;
            }

            if (ai_mesh->HasTextureCoords(0)) {
                vertex->u = ai_mesh->mTextureCoords[0][j].x;
                vertex->v = ai_mesh->mTextureCoords[0][j].y;
            }
            else {
                vertex->u = 0.0f; vertex->v = 0.0f;
            }

            outVertices.push_back(vertex);
        }

        for (unsigned int j = 0; j < ai_mesh->mNumFaces; j++)
        {
            const aiFace& ai_face = ai_mesh->mFaces[j];

            Face* face = new Face;
            std::vector<unsigned int> indices;
            std::vector<Vertex*> faceVertices;

            for (unsigned int k = 0; k < ai_face.mNumIndices; k++) {
                unsigned int vertexIndex = ai_face.mIndices[k];
                indices.push_back(vertexIndex);

                faceVertices.push_back(outVertices[vertexIndex]);
            }

            face->_indices = indices;
            face->_vertices = faceVertices;

            outFaces.push_back(face);;
        }
    }

    return true;
}