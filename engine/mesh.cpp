#include "mesh.h"

#include <iostream>
#include <math.h>
#include <algorithm>

#include <GL/glew.h>
#include <GL/freeglut.h>

static bool g_renderPaused = true;  // Start paused by default
static bool g_waitingForStep = false;
static int g_currentStepIndex = 0;
static std::vector<OctreeNode*> g_leafNodesList;  // To store all leaf nodes in order
static bool g_debugInfoPrinted = false;  // To avoid reprinting info
static bool g_holdMode = true;

extern void keyboardCallback(unsigned char key, int mouseX, int mouseY);
extern void keyboardUpCallback(unsigned char key, int mouseX, int mouseY);

static void renderText(float x, float y, const std::string& text) {
   glDisable(GL_LIGHTING);
   glDisable(GL_DEPTH_TEST);

   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   glColor3f(1.0f, 1.0f, 0.0f);  // Yellow text
   glRasterPos2f(x, y);
   for (char c : text) {
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
   }

   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
}

// Add key up handling
static void debugKeyboardUpCallback(unsigned char key, int x, int y) {
   bool isDebugKey = false;

   switch (key) {
   case 'p':
   case 's':
   case 'o':
   case 'r':
   case 'i':
      isDebugKey = true;
      break;
   }

   // Pass through non-debug keys
   if (!isDebugKey) {
      keyboardUpCallback(key, x, y);
   }
}

static void debugKeyboardCallback(unsigned char key, int x, int y) {
   bool isDebugKey = false;

   switch (key) {
   case 'p': // Toggle pause
      g_renderPaused = !g_renderPaused;
      if (g_renderPaused) {
         std::cout << ">>> RENDER PAUSED. Use 's' to step through nodes" << std::endl;
         if (g_currentStepIndex == 0) {
            g_currentStepIndex = 1;  // Start from first node
         }
      }
      else {
         std::cout << ">>> RENDER RESUMED (continuous mode)" << std::endl;
      }
      isDebugKey = true;
      break;

   case 's': // Step to next node
      if (g_renderPaused) {
         if (g_currentStepIndex < g_leafNodesList.size()) {
            g_currentStepIndex++;
            std::cout << ">>> Highlighting node " << (g_currentStepIndex - 1) << "/" << g_leafNodesList.size() << std::endl;
            glutPostRedisplay();
         }
         else if (g_currentStepIndex >= g_leafNodesList.size()) {
            std::cout << ">>> Already at last node. Press 'o' to restart from beginning." << std::endl;
         }
      }
      else {
         std::cout << ">>> Not in paused mode. Press 'p' to pause first." << std::endl;
      }
      isDebugKey = true;
      break;

   case 'o': // Reset stepping
      g_currentStepIndex = 1;  // Reset to first node
      std::cout << ">>> Reset to first node" << std::endl;
      isDebugKey = true;
      break;

   case 'r': // Resume normal rendering
      g_renderPaused = false;
      g_currentStepIndex = 0;
      std::cout << ">>> RENDER RESUMED (continuous mode)" << std::endl;
      isDebugKey = true;
      break;

   case 'i': // Print info about current node
      if (g_renderPaused && g_currentStepIndex > 0 && g_currentStepIndex <= g_leafNodesList.size()) {
         auto* node = g_leafNodesList[g_currentStepIndex - 1];
         std::cout << "\n=== Node " << (g_currentStepIndex - 1) << " of " << g_leafNodesList.size() << " ===" << std::endl;
         std::cout << "Depth: " << node->getDepth() << std::endl;
         std::cout << "Faces: " << node->getFaces().size() << std::endl;
         std::cout << "Bounds: [" << node->getLowerBounds().x << "," << node->getLowerBounds().y << "," << node->getLowerBounds().z
            << "] to [" << node->getUpperBounds().x << "," << node->getUpperBounds().y << "," << node->getUpperBounds().z << "]" << std::endl;

         // Print first few faces
         size_t maxFaces = node->getFaces().size();
         for (size_t i = 0; i < maxFaces; i++) {
            std::cout << "  Face " << i << ": ";
            for (auto* v : node->getFaces()[i]->_vertices) {
               std::cout << "(" << v->x << "," << v->y << "," << v->z << ") ";
            }
            std::cout << std::endl;
         }
         std::cout << std::endl;
      }
      else if (!g_renderPaused) {
         std::cout << ">>> Not in pause mode. Press 'p' to pause first." << std::endl;
      }
      isDebugKey = true;
      break;
   }

   // If it's not a debug key, pass it through to the original callback
   if (!isDebugKey) {
      keyboardCallback(key, x, y);
   }
}

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

    //printOctreeHierarchy(rootNode, "", false, true);
}

Mesh::~Mesh()
{

}

void Mesh::debugNodeColors()
{
   std::cout << "\n=== DEBUG: Node Color Analysis ===" << std::endl;

   std::vector<std::pair<OctreeNode*, int>> nodesWithFaces;
   std::vector<OctreeNode*> stack = { rootNode };

   while (!stack.empty()) {
      OctreeNode* node = stack.back();
      stack.pop_back();
      if (node->hasChildren()) {
         for (OctreeNode* child : node->getChildren())
            stack.push_back(child);
      }
      else if (node->hasFaces()) {
         nodesWithFaces.push_back({ node, (int)node->getFaces().size() });
      }
   }

   // Sort by face count
   std::sort(nodesWithFaces.begin(), nodesWithFaces.end(),
      [](const auto& a, const auto& b) { return a.second > b.second; });

   std::cout << "Top 10 nodes with most faces:" << std::endl;
   for (size_t i = 0; i < std::min((size_t)10, nodesWithFaces.size()); i++) {
      auto& [node, faceCount] = nodesWithFaces[i];
      glm::vec3 color = nodeColors[node];
      std::cout << "  Node " << i << ": faces=" << faceCount
         << ", depth=" << node->getDepth()
         << ", color=(" << color.r << "," << color.g << "," << color.b << ")"
         << std::endl;

      // Check if it should be red based on density calculation
      glm::vec3 expectedColor = computeDensityColor(faceCount);
      std::cout << "    Expected color: (" << expectedColor.r << "," << expectedColor.g << "," << expectedColor.b << ")" << std::endl;

      if (color != expectedColor) {
         std::cout << "    WARNING: Color mismatch!" << std::endl;
      }
   }

   std::cout << "\nNodes by depth:" << std::endl;
   for (int d = 0; d <= 10; d++) {
      bool found = false;
      for (auto& [node, faceCount] : nodesWithFaces) {
         if (node->getDepth() == d) {
            if (!found) {
               std::cout << "  Depth " << d << ":" << std::endl;
               found = true;
            }
            glm::vec3 depthColor = computeDepthColor(d);
            std::cout << "    Node: faces=" << faceCount
               << ", actual color=(" << nodeColors[node].r << ","
               << nodeColors[node].g << "," << nodeColors[node].b << ")"
               << ", depthColor=(" << depthColor.r << ","
               << depthColor.g << "," << depthColor.b << ")" << std::endl;
            break; // Only show first of each depth
         }
      }
   }
}

void Mesh::render(glm::mat4 cameraInverse)
{
   static bool callbacksRegistered = false;
   if (!callbacksRegistered && glutGetWindow() != 0) {
      // Save original callbacks
      glutKeyboardFunc(debugKeyboardCallback);
      glutKeyboardUpFunc(debugKeyboardUpCallback);
      callbacksRegistered = true;
      std::cout << ">>> Debug keyboard callbacks registered (debug keys: p, s, o, r, i)" << std::endl;
   }

   // Collect leaf nodes once (only when first needed)
   if (g_leafNodesList.empty() && rootNode) {
      std::vector<OctreeNode*> stack = { rootNode };
      while (!stack.empty()) {
         OctreeNode* node = stack.back();
         stack.pop_back();
         if (node->hasChildren()) {
            for (OctreeNode* child : node->getChildren())
               stack.push_back(child);
         }
         else if (node->hasFaces()) {
            g_leafNodesList.push_back(node);
         }
      }
      std::cout << "\n=== Octree Debug Info ===" << std::endl;
      std::cout << "Collected " << g_leafNodesList.size() << " leaf nodes for debugging" << std::endl;
      std::cout << "Starting in PAUSED mode. Press 's' to step through nodes, 'i' for info on current node" << std::endl;
      std::cout << "========================\n" << std::endl;
   }

   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixf(glm::value_ptr(cameraInverse * this->getWC()));

   glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
   glVertexPointer(3, GL_FLOAT, 0, nullptr);
   glEnableClientState(GL_VERTEX_ARRAY);

   glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
   glNormalPointer(GL_FLOAT, 0, nullptr);
   glEnableClientState(GL_NORMAL_ARRAY);

   // --- Pass 1: Render ALL nodes with their colors (solid fill) ---
   glDisable(GL_LIGHTING);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glEnable(GL_POLYGON_OFFSET_FILL);
   glPolygonOffset(1.0f, 1.0f);

   // Always render all nodes (for context)
   for (OctreeNode* node : g_leafNodesList) {
      glm::vec3& col = nodeColors[node];
      glColor3f(col.r, col.g, col.b);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nodeIndexVBOs[node]);
      glDrawElements(GL_TRIANGLES, nodeIndexCounts[node], GL_UNSIGNED_INT, nullptr);
   }

   glDisable(GL_POLYGON_OFFSET_FILL);

   // --- Pass 2: Highlight the current node (if in step mode) ---
   if (g_renderPaused && g_currentStepIndex > 0 && g_currentStepIndex <= g_leafNodesList.size()) {
      OctreeNode* currentNode = g_leafNodesList[g_currentStepIndex - 1];

      // Method 1: Wireframe overlay with thick red lines
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glLineWidth(3.0f);
      glColor3f(1.0f, 0.0f, 0.0f);  // Bright red

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nodeIndexVBOs[currentNode]);
      glDrawElements(GL_TRIANGLES, nodeIndexCounts[currentNode], GL_UNSIGNED_INT, nullptr);

      // Method 2: Draw bounding box around the node (optional, adds extra emphasis)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glLineWidth(2.0f);
      glColor3f(1.0f, 0.5f, 0.0f);  // Orange for bounding box
      
      // glClear(GL_DEPTH_BUFFER_BIT); // too see through

      glm::vec3 min = currentNode->getLowerBounds();
      glm::vec3 max = currentNode->getUpperBounds();

      // Draw bounding box
      glBegin(GL_LINES);
      // Bottom face
      glVertex3f(min.x, min.y, min.z); glVertex3f(max.x, min.y, min.z);
      glVertex3f(max.x, min.y, min.z); glVertex3f(max.x, min.y, max.z);
      glVertex3f(max.x, min.y, max.z); glVertex3f(min.x, min.y, max.z);
      glVertex3f(min.x, min.y, max.z); glVertex3f(min.x, min.y, min.z);
      // Top face
      glVertex3f(min.x, max.y, min.z); glVertex3f(max.x, max.y, min.z);
      glVertex3f(max.x, max.y, min.z); glVertex3f(max.x, max.y, max.z);
      glVertex3f(max.x, max.y, max.z); glVertex3f(min.x, max.y, max.z);
      glVertex3f(min.x, max.y, max.z); glVertex3f(min.x, max.y, min.z);
      // Vertical edges
      glVertex3f(min.x, min.y, min.z); glVertex3f(min.x, max.y, min.z);
      glVertex3f(max.x, min.y, min.z); glVertex3f(max.x, max.y, min.z);
      glVertex3f(max.x, min.y, max.z); glVertex3f(max.x, max.y, max.z);
      glVertex3f(min.x, min.y, max.z); glVertex3f(min.x, max.y, max.z);
      glEnd();
   }

   // --- Pass 3: Render all edges in black (for context) ---
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glLineWidth(1.0f);
   glColor3f(0.0f, 0.0f, 0.0f);  // Black edges

   for (OctreeNode* node : g_leafNodesList) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nodeIndexVBOs[node]);
      glDrawElements(GL_TRIANGLES, nodeIndexCounts[node], GL_UNSIGNED_INT, nullptr);
   }

   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // restore default
   glEnable(GL_LIGHTING);

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   if (g_renderPaused) {
      std::string status;
      if (g_currentStepIndex > 0 && g_currentStepIndex <= g_leafNodesList.size()) {
         auto* node = g_leafNodesList[g_currentStepIndex - 1];
         status = "STEP MODE: Node " + std::to_string(g_currentStepIndex - 1) + "/" + std::to_string(g_leafNodesList.size()) +
            " | Faces: " + std::to_string(node->getFaces().size()) +
            " | Depth: " + std::to_string(node->getDepth());
      }
      else if (g_currentStepIndex == 0) {
         status = "STEP MODE: No node selected. Press 's' to start stepping.";
      }
      else {
         status = "STEP MODE: Completed all nodes. Press 'o' to restart.";
      }
      renderText(10, 30, status);

    renderOctree(rootNode);
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

    glClear(GL_DEPTH_BUFFER_BIT);

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

OctreeNode* Mesh::getOctreeRoot() {
    return rootNode;
}