#include "mesh.h"

#include <iostream>
#include <math.h>
#include <algorithm>

#include <GL/glew.h>
#include <GL/freeglut.h>

Mesh::Mesh(std::vector<Face*> faces, std::vector<Vertex*> vertices)
{
    Mesh::Node();

    _faces = faces;
    _vertices = vertices;

    std::vector<unsigned int> _indices;
    for (Face* face : faces) {
        for (unsigned int index : face->_indices)
            _indices.push_back(index);
    }

    float* flatVertices = new float[_vertices.size() * 3];
    float* flatNormals = new float[_vertices.size() * 3];
    for (int i = 0; i < _vertices.size(); i++) {
        flatVertices[i * 3] = _vertices[i]->x;
        flatVertices[i * 3 + 1] = _vertices[i]->y;
        flatVertices[i * 3 + 2] = _vertices[i]->z;

        flatNormals[i * 3] = _vertices[i]->nx;
        flatNormals[i * 3 + 1] = _vertices[i]->ny;
        flatNormals[i * 3 + 2] = _vertices[i]->nz;
    }

    glGenBuffers(1, &vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * 3 * sizeof(float), flatVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * 3 * sizeof(float), flatNormals, GL_STATIC_DRAW);

    delete[] flatVertices;
    delete[] flatNormals;

    rootNode = new OctreeNode(_vertices, _faces);

    std::vector<OctreeNode*> stack = { rootNode };
    while (!stack.empty()) {
        OctreeNode* node = stack.back();
        stack.pop_back();
        if (node->hasChildren()) {
            for (OctreeNode* child : node->getChildren())
                stack.push_back(child);
        }
        else if (node->hasFaces()) {
            std::vector<unsigned int> indices;
            for (Face* face : node->getFaces())
                for (unsigned int idx : face->_indices)
                    indices.push_back(idx);
            unsigned int vbo;
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
            nodeIndexVBOs[node] = vbo;
            nodeIndexCounts[node] = indices.size();
            //nodeColors[node] = computeDensityColor(node->getFaces().size());
            nodeColors[node] = computeDepthColor(node->getDepth());
        }
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    printOctreeHierarchy(rootNode, "", false, true);
}

Mesh::~Mesh()
{

}

void Mesh::render(glm::mat4 cameraInverse)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(cameraInverse * this->getWC()));

    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glVertexPointer(3, GL_FLOAT, 0, nullptr);
    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glNormalPointer(GL_FLOAT, 0, nullptr);
    glEnableClientState(GL_NORMAL_ARRAY);

    // --- Pass 1: solid fill ---
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    std::vector<OctreeNode*> stack = { rootNode };
    while (!stack.empty()) {
        OctreeNode* node = stack.back();
        stack.pop_back();
        if (node->hasChildren()) {
            for (OctreeNode* child : node->getChildren())
                stack.push_back(child);
        }
        else if (node->hasFaces()) {
            glm::vec3& col = nodeColors[node];
            glColor3f(col.r, col.g, col.b);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nodeIndexVBOs[node]);
            glDrawElements(GL_TRIANGLES, nodeIndexCounts[node], GL_UNSIGNED_INT, nullptr);
        }
    }
    glDisable(GL_POLYGON_OFFSET_FILL);

    // --- Pass 2: edges only ---
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    stack = { rootNode };
    while (!stack.empty()) {
        OctreeNode* node = stack.back();
        stack.pop_back();
        if (node->hasChildren()) {
            for (OctreeNode* child : node->getChildren())
                stack.push_back(child);
        }
        else if (node->hasFaces()) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nodeIndexVBOs[node]);
            glDrawElements(GL_TRIANGLES, nodeIndexCounts[node], GL_UNSIGNED_INT, nullptr);
        }
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // restore default
    glEnable(GL_LIGHTING);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    //renderOctree(rootNode);
}

void Mesh::renderOctree(OctreeNode* root)
{
    std::vector<OctreeNode*> stack = { root };
    std::vector<OctreeNode*> leafNodes;

    while (!stack.empty()) {
        OctreeNode* node = stack.back();
        stack.pop_back();
        if (node->hasFaces())
            leafNodes.push_back(node);
        if (node->hasChildren())
            for (OctreeNode* child : node->getChildren())
                stack.push_back(child);
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);

    static const int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    for (OctreeNode* node : leafNodes) {
        if (node->getFaces().size() >= 10) continue;
        glm::vec3 mn = node->getLowerBounds();
        glm::vec3 mx = node->getUpperBounds();
        glm::vec3 c[8] = {
            {mn.x,mn.y,mn.z},{mx.x,mn.y,mn.z},{mx.x,mx.y,mn.z},{mn.x,mx.y,mn.z},
            {mn.x,mn.y,mx.z},{mx.x,mn.y,mx.z},{mx.x,mx.y,mx.z},{mn.x,mx.y,mx.z}
        };
        for (auto& e : edges)
            glVertex3fv(&c[e[0]].x), glVertex3fv(&c[e[1]].x);
    }
    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    for (OctreeNode* node : leafNodes) {
        if (node->getFaces().size() < 10) continue;
        glm::vec3 mn = node->getLowerBounds();
        glm::vec3 mx = node->getUpperBounds();
        glm::vec3 c[8] = {
            {mn.x,mn.y,mn.z},{mx.x,mn.y,mn.z},{mx.x,mx.y,mn.z},{mn.x,mx.y,mn.z},
            {mn.x,mn.y,mx.z},{mx.x,mn.y,mx.z},{mx.x,mx.y,mx.z},{mn.x,mx.y,mx.z}
        };
        for (auto& e : edges)
            glVertex3fv(&c[e[0]].x), glVertex3fv(&c[e[1]].x);
    }
    glEnd();

    glPopAttrib();
}

void Mesh::printOctreeHierarchy(OctreeNode* node, const std::string& prefix, bool isLast, bool isRoot) {
    if (!node) return;
    std::cout << prefix;
    if (!isRoot)
        std::cout << (isLast ? "|__ " : "|-- ");

    if (node->hasChildren())
        std::cout << "[Branch] depth: " << node->getDepth() << std::endl;
    else
        std::cout << "[Leaf] depth: " << node->getDepth() << " faces: " << node->getFaces().size() << std::endl;

    std::string childPrefix = prefix + (isRoot ? "" : (isLast ? "    " : "|   "));
    std::vector<OctreeNode*> children = node->getChildren();
    int childCount = children.size();
    for (int i = 0; i < childCount; i++)
        printOctreeHierarchy(children[i], childPrefix, i == childCount - 1, false);
}

glm::vec3 Mesh::computeDensityColor(size_t faceCount) {
    const float maxFaces = 10.0f;
    float t = std::min((float)faceCount / maxFaces, 1.0f);

    glm::vec3 colors[] = {
        {0.0f, 0.0f, 1.0f},  // blue     t=0.00
        {0.0f, 1.0f, 1.0f},  // cyan     t=0.25
        {0.0f, 1.0f, 0.0f},  // green    t=0.50
        {1.0f, 1.0f, 0.0f},  // yellow   t=0.75
        {1.0f, 0.0f, 0.0f},  // red      t=1.00
    };
    int segments = 4;
    float scaled = t * segments;
    int idx = std::min((int)scaled, segments - 1);
    float frac = scaled - idx;

    return glm::mix(colors[idx], colors[idx + 1], frac);
}

glm::vec3 Mesh::computeDepthColor(int depth)
{
    const float maxDepth = 10.0f;
    float t = std::min((float)depth / maxDepth, 1.0f);

    glm::vec3 colors[] = {
        {0.0f, 0.0f, 1.0f},  // blue     t=0.00
        {0.0f, 1.0f, 1.0f},  // cyan     t=0.25
        {0.0f, 1.0f, 0.0f},  // green    t=0.50
        {1.0f, 1.0f, 0.0f},  // yellow   t=0.75
        {1.0f, 0.0f, 0.0f},  // red      t=1.00
    };
    int segments = 4;
    float scaled = t * segments;
    int idx = std::min((int)scaled, segments - 1);
    float frac = scaled - idx;

    return glm::mix(colors[idx], colors[idx + 1], frac);
}