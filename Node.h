#ifndef NODE_H
#define NODE_H

#include <vector>
#include <string>

template <typename T> class Node {
    public:        
        T data;
        std::string edge_label; // Edge Label for incoming node
        
        Node<T>* parent;
        std::vector<Node<T>*> children;

        // No-argument constructor
        Node() : data(), edge_label(""), parent(nullptr) {}

        /*  Parameterized constructor
            - const: don't modify inputs
            - &: pass by reference, don't create a new copy
            - accept literal
            - edge_label = "", default argument*/
        Node(const T& data, const std::string& edge_label = "") : data(data), edge_label(edge_label), parent(nullptr) {}

        // Destructor
        ~Node() {
            for (Node<T>* child : children) {
                delete child;
            }
        }

        // Add a child node 
        void addChild(Node<T>* child) {
            child->parent = this;
            children.push_back(child);
        }
};

#endif