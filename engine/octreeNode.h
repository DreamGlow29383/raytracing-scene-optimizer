#include <stdbool.h>
#include <vector>
#include "engine.h"
#include "face.h"

static int max_faces = 10;
static int max_depth = 10;

class OctreeNode {
public:
    OctreeNode(std::vector<Vertex*> allVertices, std::vector<Face*> allFaces);
    OctreeNode(glm::vec3 lowerCorner, glm::vec3 upperCorner, int depth);
    ~OctreeNode();

    void insert(Face* face);

    // check if vertices are within bounding box
    bool check(const Face* face);
    bool isSplit();
    void render(glm::mat4 cameraInverse);

    glm::vec3 getLowerBounds() const { return lowerBoundsCorner; }
    glm::vec3 getUpperBounds() const { return upperBoundsCorner; }
    std::vector<OctreeNode*> getChildren() const { return children; }
    bool hasChildren() const { return !children.empty(); }
    std::vector<Face*> getFaces() const { return faces; }
    bool hasFaces() const { return !faces.empty(); }
    int getDepth() const { return node_depth; }
private:

    void split();
    bool m_isSplit = false;
    std::vector<Face*> faces;

   /* children assigned in order left to right, front to back, bottom to top
       -------------
      /__7__/__6__ /|
     /     /      / |
     -------------  |
    |  3  | / 2  |  |
    |_____|/_____|  |
    |  0  | / 1  | /
    |_____|/_____|/      */
    std::vector<OctreeNode*> children;
    glm::vec3 lowerBoundsCorner;
    glm::vec3 upperBoundsCorner;
    int node_depth = 0;
};