#include <stdbool.h>
#include <vector>
#include "engine.h"
#include "face.h"

static int max_faces = 10;

class OctreeNode {
public:
    OctreeNode(std::vector<Vertex*> allVertices, std::vector<Face*> allFaces);
    OctreeNode(glm::vec3 lowerCorner, glm::vec3 upperCorner);
    ~OctreeNode();

    void insert(Face* face);

    // check if vertices are within bounding box
    bool check(const Face* face);
    bool isSplit();
    void render(glm::mat4 cameraInverse);

private:

    void split();
    bool m_isSplit = false;
    std::vector<Face*> faces;

    /* bounding box defined left to right, front to back, bottom to top
       7------------6
      /            /|
     /            / |
    3------------2  |
    |            |  |
    |            |  5
    |            | /
    0____________1/       */
   std::vector<glm::vec3> m_bounding_box_corners;

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
};