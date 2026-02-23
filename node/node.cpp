#include <vector>

int max_triangles_per_node = 10;

class Node {
    public:
        void const add(Node const& n);
        std::vector<Node*> split();

    private:
        std::vector<Node*> children;
        std::vector<Vertex_Buffer*> ids;
};

class Vertex_Buffer {

};


void const Node::add(Node const& n) {
    
}

std::vector<Node*> split() {
    
}