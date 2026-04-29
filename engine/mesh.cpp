#include "mesh.h"

#include <iostream>
#include <math.h>
#include <algorithm>

#include <GL/glew.h>
#include <GL/freeglut.h>

Mesh::Mesh(std::vector<Face*> faces, std::vector<Vertex*> vertices)
{
    Mesh::Node();

    generateMesh(faces, vertices);

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
            _leafNodes.push_back(node);
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
            faceColors[node] = computeDensityColor(node->getFaces().size());
            depthColors[node] = computeDepthColor(node->getDepth());
            nodeColors[node] = computeRandomColor();
        }
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    std::vector<unsigned int> allIndices;
    for (OctreeNode* node : _leafNodes) {
        for (Face* face : node->getFaces()) {
            for (unsigned int idx : face->_indices) {
                allIndices.push_back(idx);
            }
        }
    }
    glGenBuffers(1, &unifiedIndexVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unifiedIndexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, allIndices.size() * sizeof(unsigned int), allIndices.data(), GL_STATIC_DRAW);
    unifiedIndexCount = allIndices.size();

    buildExpandedBuffers();
    updateColorVBO(0);
}

Mesh::Mesh(std::vector<Face*> faces, std::vector<Vertex*> vertices, OctreeNode* octreeRoot) {
    Mesh::Node();

    generateMesh(faces, vertices);

    rootNode = octreeRoot;

    std::vector<OctreeNode*> stack = { rootNode };
    while (!stack.empty()) {
        OctreeNode* node = stack.back();
        stack.pop_back();
        if (node->hasChildren()) {
            for (OctreeNode* child : node->getChildren())
                stack.push_back(child);
        }
        else if (node->hasFaces()) {
            _leafNodes.push_back(node);
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
            faceColors[node] = computeDensityColor(node->getFaces().size());
            depthColors[node] = computeDepthColor(node->getDepth());
            nodeColors[node] = computeRandomColor();
        }
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    buildExpandedBuffers();
    updateColorVBO(0);
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &vertexVBO);
    glDeleteBuffers(1, &normalVBO);
    glDeleteBuffers(1, &expandedVertexVBO);
    glDeleteBuffers(1, &expandedNormalVBO);
    glDeleteBuffers(1, &colorVBO);
    glDeleteBuffers(1, &unifiedIndexVBO);
}

void Mesh::render(glm::mat4 cameraInverse)
{
    Eng::Base& eng = Eng::Base::getInstance();

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(cameraInverse * this->getWC()));

    glBindBuffer(GL_ARRAY_BUFFER, expandedVertexVBO);
    glVertexPointer(3, GL_FLOAT, 0, nullptr);
    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, expandedNormalVBO);
    glNormalPointer(GL_FLOAT, 0, nullptr);
    glEnableClientState(GL_NORMAL_ARRAY);

    if (eng.getColoringMode() != 0) {
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glColorPointer(3, GL_FLOAT, 0, nullptr);
        glEnableClientState(GL_COLOR_ARRAY);

        glDisable(GL_LIGHTING);
    }
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (eng.getColoringMode() == 0) {
        GLfloat material_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_diffuse);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unifiedIndexVBO);
    glDrawElements(GL_TRIANGLES, unifiedIndexCount, GL_UNSIGNED_INT, nullptr);

    if (eng.getColoringMode() != 0) {
        glDisableClientState(GL_COLOR_ARRAY);
        glEnable(GL_LIGHTING);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    if (eng.getShowNodeBoundaries())
        renderOctree(rootNode);
}

void Mesh::renderOctree(OctreeNode* root)
{
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
    for (OctreeNode* node : _leafNodes) {
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

    glClear(GL_DEPTH_BUFFER_BIT);

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    for (OctreeNode* node : _leafNodes) {
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
    const float maxFaces = MAX_FACES;
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
    const float maxDepth = MAX_DEPTH;
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

glm::vec3 Mesh::computeRandomColor()
{
    float r = (std::rand() % 256) / 255.0f;
    float g = (std::rand() % 256) / 255.0f;
    float b = (std::rand() % 256) / 255.0f;
    return glm::vec3(r, g, b);
}

OctreeNode* Mesh::getOctreeRoot() {
    return rootNode;
}

void Mesh::generateMesh(std::vector<Face*> faces, std::vector<Vertex*> vertices) {
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
}

void Mesh::updateColorVBO(int coloringMode) {
    std::vector<float> colors(unifiedIndexCount * 3, 0.8f);

    if (coloringMode != 0) {
        for (OctreeNode* node : _leafNodes) {
            glm::vec3 col = (coloringMode == 1) ? depthColors[node]
                : (coloringMode == 2) ? faceColors[node]
                : nodeColors[node];
            const NodeRange& r = nodeRanges[node];
            for (unsigned int i = r.start; i < r.start + r.count; i++) {
                colors[i * 3] = col.r;
                colors[i * 3 + 1] = col.g;
                colors[i * 3 + 2] = col.b;
            }
        }
    }

    if (colorVBO == 0) glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::buildExpandedBuffers() {
    std::vector<float> exVertices;
    std::vector<float> exNormals;
    std::vector<unsigned int> allIndices;

    exVertices.reserve(_faces.size() * 3 * 3);
    exNormals.reserve(_faces.size() * 3 * 3);
    allIndices.reserve(_faces.size() * 3);

    unsigned int cursor = 0;

    for (OctreeNode* node : _leafNodes) {
        unsigned int rangeStart = cursor;

        for (Face* face : node->getFaces()) {
            for (unsigned int oldIdx : face->_indices) {
                Vertex* v = _vertices[oldIdx];
                exVertices.push_back(v->x);
                exVertices.push_back(v->y);
                exVertices.push_back(v->z);
                exNormals.push_back(v->nx);
                exNormals.push_back(v->ny);
                exNormals.push_back(v->nz);
                allIndices.push_back(cursor++);
            }
        }

        nodeRanges[node] = { rangeStart, cursor - rangeStart };
    }

    glGenBuffers(1, &expandedVertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, expandedVertexVBO);
    glBufferData(GL_ARRAY_BUFFER, exVertices.size() * sizeof(float), exVertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &expandedNormalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, expandedNormalVBO);
    glBufferData(GL_ARRAY_BUFFER, exNormals.size() * sizeof(float), exNormals.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &unifiedIndexVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unifiedIndexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, allIndices.size() * sizeof(unsigned int), allIndices.data(), GL_STATIC_DRAW);
    unifiedIndexCount = allIndices.size();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}